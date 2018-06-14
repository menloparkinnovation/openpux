
//
// TODO:
//
// See documentation for "PARAMETERS:" in menlo_cnc_asm.h
//
// Implement parameters base number system support.
//
// Add basic include file support. This is to allow a common
// machine configuration block to be included in assembler programs.
//

//
// menlo_cnc_asm.c - Assembler for menlo_cnc, FPGA based machine tool controller.
//
// Copyright (c) 2018 Menlo Park Innovation LLC
//
// https://github.com/menloparkinnovation/openpux/tree/master/menlocnc
//
// 06/06/2018
//

//
//   menlo_cnc - A Menlo Park Innovation LLC Creation.
//
//   Copyright (C) 2018 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Snapshot License
//
//   This license is for a specific snapshot of a base work of
//   Menlo Park Innovation LLC on a non-exclusive basis with no warranty
//   or obligation for future updates. This work, any portion, or derivative
//   of it may be made available under other license terms by
//   Menlo Park Innovation LLC without notice or obligation to this license.
//
//   There is no warranty, statement of fitness, statement of
//   fitness for any purpose, and no statements as to infringements
//   on any patents.
//
//   Menlo Park Innovation has no obligation to offer support, updates,
//   future revisions and improvements, source code, source code downloads,
//   media, etc.
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//   This specific snapshot is made available under the following license:
//
//   The MIT license:
//
//   https://opensource.org/licenses/MIT
//
//   A copy has been provided here:
//
//   The MIT License (MIT)
//   Copyright (c) 2018 Menlo Park Innovation LLC
// 
//   Permission is hereby granted, free of charge, to any person obtaining a
//   copy of this software and associated documentation files (the "Software"),
//   to deal in the Software without restriction, including without limitation
//   the rights to use, copy, modify, merge, publish, distribute, sublicense,
//   and/or sell copies of the Software, and to permit persons to whom the
//   Software is furnished to do so, subject to the following conditions:
// 
//   The above copyright notice and this permission notice shall be included in
//   all copies or substantial portions of the Software.
// 
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//   DEALINGS IN THE SOFTWARE.
//
//   This is taken from LinuxCNC and applies here:
//
//   https://github.com/LinuxCNC/linuxcnc/blob/master/README.md
//
//   THE AUTHORS OF THIS SOFTWARE ACCEPT ABSOLUTELY NO LIABILITY FOR ANY HARM OR LOSS RESULTING FROM ITS USE.
// 
//   IT IS EXTREMELY UNWISE TO RELY ON SOFTWARE ALONE FOR SAFETY.
// 
//   Any machinery capable of harming persons must have provisions for completely
//   removing power from all motors, etc, before persons enter any danger area.
// 
//   All machinery must be designed to comply with local and national safety
//   codes, and the authors of this software can not, and do not, take any
//   responsibility for such compliance.
// 

#include <stdio.h>
#include <errno.h>
#include <stdlib.h> // exit
#include <limits.h> // LONG_MAX, LONG_MIN
#include <string.h> // strtok, bzero

// Include binary type definitions for menlo_cnc
#include "menlo_cnc.h"

#include "menlo_cnc_asm.h"

//
// Assembler definition:
//
// The assembler commands machine tool axis. Standard assembler syntax
// leads with the instruction/opcode first since its implied to operate
// against the single processor execution stream.
//
// In this case the axis, or object being operated against is specified
// first, then the instruction, then the instructions parameters in
// the standard manner.
//
// Since machine tool axis motion must be co-ordinated in real time, the
// assembler represents blocks of commands in which the instruction on
// each specified axis is executed in parallel, and retired/completed in
// parallel. Execution of the block is not complete until all axis
// instructions in the block complete.
//
// The execution state of the machine is from block to block, and not
// instruction to instruction as is typical of a Single Instruction,
// Single Data operand processor. It can be called a block or packet
// oriented multiple instruction, multiple data machine, or a longword
// microcode which controls parallel hardware execution units.
//
// Valid assembler block formats:
//
// Multi-block with explicit begin/end:
//
// begin
//   X, CW,   100, 16, 4
//   Y, CCW,   50,  8, 2
//   Z, DWELL, 75, 12, 0
//   A, NOP,    0,  0, 0
// end
//
// Single block with implied begin/end:
//
// X,CW,100,16,4, Y,CCW,50,8,2, Z,DWELL,75,12,0, A,NOP,0,0,0
//
// begin, end block on a single line:
//
// begin, X,CW,100,16,4, Y,CCW,50,8,2, Z,DWELL,75,12,0, A,NOP,0,0,0, end
//

//
// Setting this to 1 shows the state machine of the assembler
// to assist debugging.
//
#define DBG_PRINT_ENABLED 1

#if DBG_PRINT_ENABLED
#define DBG_PRINT(x)             (printf(x))
#define DBG_PRINT1(x, arg)       (printf(x, arg))
#define DBG_PRINT2(x, arg, arg2) (printf(x, arg, arg2))
#define DBG_PRINT3(x, arg, arg2, arg3) (printf(x, arg, arg2, arg3))
#else
#define DBG_PRINT(x)
#define DBG_PRINT1(x, arg)
#define DBG_PRINT2(x, arg, arg2)
#define DBG_PRINT3(x, arg, arg2, arg3)
#endif

//
// Forward references.
//
// Prototypes
//

int processSymbol(PASSEMBLER_CONTEXT context, char* s);

char* copyNewString(char* s);

void freeString(char* s);

char* stripLeadingWhiteSpace(char *s);

char* stripTrailingWhiteSpace(char *old);

void reset_assembler_context(PASSEMBLER_CONTEXT context);

int process_header(PASSEMBLER_CONTEXT context, char* line);

int process_config(PASSEMBLER_CONTEXT context, char* line);

int process_begin_line(PASSEMBLER_CONTEXT context);

int process_end_of_line(PASSEMBLER_CONTEXT context);

int process_generic_symbol(PASSEMBLER_CONTEXT context, char* symbol);

int process_begin_block(PASSEMBLER_CONTEXT context, char* s);

int process_end_block(PASSEMBLER_CONTEXT context, char* s);

int process_assembly_block(PASSEMBLER_CONTEXT context);

PAXIS_OPCODE get_axis_opcode(PASSEMBLER_CONTEXT context);

void reset_current_assembler_axis(PASSEMBLER_CONTEXT context);

void reset_current_assembler_opcode(PASSEMBLER_CONTEXT context);

enum AxisState get_axis_state_by_symbol(PASSEMBLER_CONTEXT context, char* symbol);

int axis_symbol_to_binary(char* s);

int opcode_symbol_to_binary(char* s);

int string_to_unsigned_long(char* s, unsigned long* l);

int process_opcode(PASSEMBLER_CONTEXT context, PAXIS_OPCODE op, char* symbol);

int processLines(PASSEMBLER_CONTEXT context, FILE* f);

int
compile_assembly_block(
    PASSEMBLER_CONTEXT context,
    POPCODE_BLOCK_FOUR_AXIS symbolic,
    POPCODE_BLOCK_FOUR_AXIS_BINARY bin
    );

int
compile_axis_opcode(
    PASSEMBLER_CONTEXT context,
    PAXIS_OPCODE symbolic,
    PAXIS_OPCODE_BINARY bin
    );

//
// Process an assembler line.
//
// An assembler line may represent a complete block, or
// a portion of one.
//
// It may also represent a comment, etc.
//
// Caller allocated and frees string.
//
// Returns: 
//   0 - Success
//   !0 - Error code based on errno
//
int
process_assembler_line(PASSEMBLER_CONTEXT context, char* line)
{
  int ret;
  char *saveptr;
  char *s;
  char *news;

  // Strip leading whitespace
  line = stripLeadingWhiteSpace(line);

  // comment, ignore rest of line.
  if (line[0] == ';') {
    return 0;
  }

  // empty line
  if (line[0] == '\0') {
    return 0;
  }

  saveptr = NULL;

  s = strtok_r(line, ",", &saveptr);
  if (s == NULL) {
    return 0;
  }

  s = stripLeadingWhiteSpace(s);

  news = stripTrailingWhiteSpace(s);

  ret = process_begin_line(context);

  ret = processSymbol(context, news);

  free(news);

  if (ret != 0) {
    return ret;
  }

  //
  // Continue till NULL returned
  //
  while ((s = strtok_r(NULL, ",", &saveptr)) != NULL) {

    s = stripLeadingWhiteSpace(s);
    news = stripTrailingWhiteSpace(s);

    if (news[0] == '\0') {
      printf("internal error, unexpected empty token line %d\n", context->lineNumber);
      return ENOTSUP;
    }

    ret = processSymbol(context, news);
    free(news);

    if (ret != 0) {
      return ret;
    }
  }

  // End of line reached.
  ret = process_end_of_line(context);

  return ret;
}

//
// Process a symbol.
//
// A stream of symbols stripped of whitespace and delimiters is passed to
// this routine. The state machine is stored in context.
//
// context - context for assembly session
//
// symbol - symbol stripped of any whitespace, delimiters.
//
// Note: Caller frees the memory allocated for symbol.
//
int
processSymbol(PASSEMBLER_CONTEXT context, char* symbol)
{
  //
  // Main state machine handles header, config, and begin, end blocks.
  //

  DBG_PRINT1("    processing symbol :%s:\n", symbol);

  if (strcmp(symbol, OPCODE_BEGIN_SYMBOL) == 0) {

    if ((context->saw_header == 0) || (context->saw_config == 0)) {
      printf("Line %d: Opcodes can only be specified after header and config statements\n", context->lineNumber);
      return EBADR;
    }

    if (context->saw_end != 0) {
      printf("Line %d: Opcode begin can only be specified after a balanced end\n", context->lineNumber);
      return EBADR;
    }

    return process_begin_block(context, symbol);
  }
  else if (strcmp(symbol, OPCODE_END_SYMBOL) == 0) {
    if (context->saw_begin == 0) {
      printf("Line %d: Opcode end can only be specified after a balanced begin\n", context->lineNumber);
      return EBADR;
    }

    return process_end_block(context, symbol);
  }
  else {
    // process generic/nonspecific symbol
    return process_generic_symbol(context, symbol);
  }

  // Falloff
  return ENOTSUP;
}

char* stripLeadingWhiteSpace(char *s)
{

  while(s[0] != '\0') {

    switch(s[0]) {

      case ' ':
        s++;
        break;

      case '\t':
        s++;
        break;

      case '\r':
        s++;
        break;

      case '\n':
        s++;
        break;

      default:
        return s;
    }
  }

  return s;
}

char *copyNewString(char* s)
{
  char* news;
  size_t len;
  
  len = strlen(s);
  news = (char*)malloc(len + 1);

  strcpy(news, s);

  return news;
}

void freeString(char* s)
{
  free(s);
}

//
// Strip trailing whitespace.
//
// Since it modifies the buffer a new string is returned.
//
// There must be no leading white space. Use stringLeadingWhiteSpace
// on the string before calling this function.
//
char* stripTrailingWhiteSpace(char *old)
{
  char* s;
  char* news;
  size_t len;

  len = strlen(old);
  news = (char*)malloc(len + 1);

  strcpy(news, old);

  s = news;

  while(s[0] != '\0') {

    switch(s[0]) {

      case ' ':
        s[0] = '\0';
        return news;

      case '\t':
        s[0] = '\0';
        return news;

      case '\r':
        s[0] = '\0';
        return news;

      case '\n':
        s[0] = '\0';
        return news;

      default:
        s++;
        break;
    }
  }

  return news;
}

int
process_header(PASSEMBLER_CONTEXT context, char* symbol)
{
  DBG_PRINT2("    Line %d: process_header: %s\n", context->lineNumber, symbol);

  context->saw_header = 1;

  //printf("Line %d: error processing header\n", context->lineNumber);

  //
  // Need to process header arguments.
  //
  // Could get the rest of the line, or get them a token at a time.
  //
  // Could have a call for end of line.
  //
  // begin/end is not allowed.
  //
  // 3 arguments till end of line.
  //

  return 0;
}

int
process_config(PASSEMBLER_CONTEXT context, char* symbol)
{
  DBG_PRINT2("    Line %d: process_config: %s\n", context->lineNumber, symbol);

  context->saw_config = 1;

  //
  // Same processing requirments as header.
  //

  return 0;
}

int
process_begin_block(PASSEMBLER_CONTEXT context, char* symbol)
{
  DBG_PRINT1("*** BEGIN BLOCK *** Line %d\n", context->lineNumber);

  //
  // new block
  //
  // could be start of the current line, or stand alone.
  //
  // Don't know until next tokens are retrieved.
  //

  context->saw_begin = 1;

  return 0;
}

int
process_end_block(PASSEMBLER_CONTEXT context, char* symbol)
{
  int ret;

  DBG_PRINT1("*** END BLOCK *** Line %d\n", context->lineNumber);

  context->saw_end = 1;

  ret = process_assembly_block(context);

  // Reset the current block to begin a new one.
  reset_assembler_context(context);

  return ret;
}

int
process_begin_line(PASSEMBLER_CONTEXT context)
{
  DBG_PRINT1("    *** BEGIN_LINE *** Line %d\n", context->lineNumber);

  return 0;
}

int
process_end_of_line(PASSEMBLER_CONTEXT context)
{
  int ret;

  DBG_PRINT1("    *** END_OF_LINE *** Line %d\n", context->lineNumber);

  //
  // Automatically close implied end block of a single line.
  //
  if (context->single_line_block != 0) {

    if (context->saw_end != 0) {
      printf("process_end_of_line: saw_end set, bad sequence Line %d\n", context->lineNumber);
      return EBADF;
    }

    if (context->saw_begin == 0) {
      printf("process_end_of_line: saw_begin not set, bad sequence Line %d\n", context->lineNumber);
      return EBADF;
    }

    DBG_PRINT1("    Line %d: implied end block\n", context->lineNumber);

    ret = process_end_block(context, (char*)"end");
    if (ret != 0) {
      return ret;
    }
  }

  return 0;
}

//
// Process a generic symbol.
//
// Symbols arrive one by one stripped of whitespace.
//
// State machines for begin/end blocks, begin/end line, axis, opcode,
// and arguments track the expected symbol stream.
//
// Example symbol sequences from valid assembly blocks:
//
// Implied begin/end block handled by begin/end line state machine:
//
// X,CW,100,16,4, Y,CCW,50,8,2, Z,DWELL,75,12,0, A,NOP,0,0,0
//
// Specific begin/end block handled by begin/end blocks state machine:
//
// begin, X,CW,100,16,4, Y,CCW,50,8,2, Z,DWELL,75,12,0, A,NOP,0,0,0, end
//
// Long format begin/end block handled by block and line state machines:
//
// begin
//   X,CW,100,16,4
//   Y,CCW,50,8,2
//   Z,DWELL,75,12,0
//   A,NOP,0,0,0
// end
//
int
process_generic_symbol(PASSEMBLER_CONTEXT context, char* symbol)
{
  int ret;

  DBG_PRINT2("    Line %d: process_generic_symbol: %s\n", context->lineNumber, symbol);

  if (context->saw_end != 0) {
    // bad sequence
    printf("process_generic_symbol: saw_end set, bad sequence Line %d\n", context->lineNumber);
    return EBADR;
  }

  if (context->saw_begin == 0) {

    // It's an implied begin block
    DBG_PRINT2("    Line %d: implied begin block symbol %s\n", context->lineNumber, symbol);

    ret = process_begin_block(context, (char*)"begin");
    if (ret != 0) {
      // State error
      return ret;
    }

    // Record it for end-of-line handling.
    context->single_line_block = 1;
  }

  //
  // Track processing state
  //
  // Axis
  // Opcode
  // Arg0
  // Arg1
  // Arg3
  //

  //
  // Determine the state of assembly of the instruction.
  //
  switch(context->opcode_state) {

  case OpCodeStateIdle:

    //
    // Axis state should be idle.
    //
    if (context->axis_state != AxisStateIdle) {
      printf("invalid axis state at opcode start %d, symbol %s\n", context->lineNumber, symbol);
      return EBADF;
    }

    //
    // On success this sets axis_state to the axis entries enum value.
    //
    context->axis_state = get_axis_state_by_symbol(context, symbol);
    if (context->axis_state == AxisStateUnknown) {
      context->axis_state = AxisStateIdle;
      printf("invalid axis line %d\n", context->lineNumber);
      return EBADR;
    }

    //
    // If Idle, the first symbol represents an axis so get its
    // opcode assembly struct from the current axis state just set.
    //
    context->current_axis_opcode = get_axis_opcode(context);
    if (context->current_axis_opcode == NULL) {
      printf("invalid opcode axis line %d\n", context->lineNumber);
      return EBADF;
    }

    context->current_axis_opcode->axis = copyNewString(symbol);
    if (context->current_axis_opcode->axis == NULL) {
      printf("out of memory assembling axis opcode line %d\n", context->lineNumber);
      return ENOMEM;
    }

    DBG_PRINT1("    AXIS: %s\n", symbol);

    // Set the OpCode state, the OpCode symbol follows the axis symbol.
    context->opcode_state = OpCodeStateAxis;

    break;

  case OpCodeStateAxis:

    //
    // Current symbol is the opcode/instruction
    //
    // This sets context->current_axis_opcode->opcode on success.
    //
    ret = process_opcode(context, context->current_axis_opcode, symbol);
    if (ret != 0) {
      printf("invalid opcode/instruction line %d\n", context->lineNumber);
      return ret;
    }

    DBG_PRINT1("    OPCODE: %s\n", symbol);

    context->opcode_state = OpCodeStateInstruction;
    break;

  case OpCodeStateInstruction:

    //
    // Current symbol is the first argument Arg0
    //
    // Note: Arguments are validated at binary assembly
    // time and not here since they are specific to binary instruction
    // encodings, range, etc. At this point any string value is
    // stored as the argument.
    //

    context->current_axis_opcode->arg0 = copyNewString(symbol);
    if (context->current_axis_opcode->arg0 == NULL) {
      printf("out of memory assembling opcode arg0 line %d\n", context->lineNumber);
      return ENOMEM;
    }

    DBG_PRINT1("    ARG0: %s\n", symbol);

    context->opcode_state = OpCodeStateArg0;
    break;

  case OpCodeStateArg0:

    //
    // Current symbol is the second argument Arg1
    //

    context->current_axis_opcode->arg1 = copyNewString(symbol);
    if (context->current_axis_opcode->arg1 == NULL) {
      printf("out of memory assembling opcode arg1 line %d\n", context->lineNumber);
      return ENOMEM;
    }

    DBG_PRINT1("    ARG1: %s\n", symbol);

    context->opcode_state = OpCodeStateArg1;
    break;

  case OpCodeStateArg1:

    //
    // Current symbol is the third argument Arg2
    //

    context->current_axis_opcode->arg2 = copyNewString(symbol);
    if (context->current_axis_opcode->arg2 == NULL) {
      printf("out of memory assembling opcode arg2 line %d\n", context->lineNumber);
      return ENOMEM;
    }

    DBG_PRINT1("    ARG2: %s\n", symbol);

    //
    // Have the whole instruction and arguments stored in the
    // axis entry of the multiiple axis opcode symbols block.
    //
    // Get ready for the next axis and opcode in the block.
    //
    context->opcode_state = OpCodeStateIdle;

    context->current_axis_opcode = NULL;

    context->axis_state = AxisStateIdle;

    DBG_PRINT1("    *** FINISHED AXIS,OPCODE Back to IDLE: %s\n", symbol);

    break;

  case OpCodeStateArg2:
  case OpCodeStateComplete:
    // fallthrough

  default:
    printf("bad opcode state line %d", context->lineNumber);
    return EBADF;
  }

  return 0;
}

//
// Get the axis state by symbol.
//
enum AxisState
get_axis_state_by_symbol(PASSEMBLER_CONTEXT context, char* symbol)
{
  enum AxisState axis = AxisStateUnknown;

  if (strcmp(symbol, X_AXIS_SYMBOL) == 0) {
    axis = AxisStateX;
  }
  else if (strcmp(symbol, Y_AXIS_SYMBOL) == 0) {
    axis = AxisStateY;
  }
  else if (strcmp(symbol, Z_AXIS_SYMBOL) == 0) {
    axis = AxisStateZ;
  }
  else if (strcmp(symbol, A_AXIS_SYMBOL) == 0) {
    axis = AxisStateA;
  }
  else if (strcmp(symbol, B_AXIS_SYMBOL) == 0) {
    axis = AxisStateB;
  }
  else if (strcmp(symbol, C_AXIS_SYMBOL) == 0) {
    axis = AxisStateC;
  }
  else if (strcmp(symbol, U_AXIS_SYMBOL) == 0) {
    axis = AxisStateU;
  }
  else if (strcmp(symbol, V_AXIS_SYMBOL) == 0) {
    axis = AxisStateV;
  }
  else if (strcmp(symbol, W_AXIS_SYMBOL) == 0) {
    axis = AxisStateW;
  }
  else if (strcmp(symbol, INFO_AXIS_SYMBOL) == 0) {
    axis = AxisStateInfo;
  }
  else {
    printf("unrecognized axis %s\n", symbol);
    axis = AxisStateUnknown;
  }

  return axis;
}

//
// Return the OpCode entry for the axis represented by symbol.
//
PAXIS_OPCODE
get_axis_opcode(PASSEMBLER_CONTEXT context)
{

  //
  // Determine which axis/resource whose instruction is currently being assembled.
  //
  switch(context->axis_state) {

  case AxisStateX:
    return &context->opcode_block.x;
    break;

  case AxisStateY:
    return &context->opcode_block.y;
    break;

  case AxisStateZ:
    return &context->opcode_block.z;
    break;

  case AxisStateA:
    return &context->opcode_block.a;
    break;

  case AxisStateInfo:
    return &context->opcode_block.info;
    break;

  case AxisStateB:

    // fall through for unsupported axis right now

  case AxisStateC:

  case AxisStateU:

  case AxisStateV:

  case AxisStateW:

  case AxisStateIdle:
    // fallthrough

  default:
    printf("bad axis_state for opcode line %d", context->lineNumber);
    return NULL;
  }

  return NULL;
}

//
// Process an axis opcode
//
int
process_opcode(PASSEMBLER_CONTEXT context, PAXIS_OPCODE op, char* symbol)
{

  if (strcmp(symbol, OPCODE_NOP_SYMBOL) == 0) {
    // copy new string
    op->opcode = copyNewString(symbol);
  }
  else if (strcmp(symbol, OPCODE_MOTION_CW_SYMBOL) == 0) {
    op->opcode = copyNewString(symbol);
  }
  else if (strcmp(symbol, OPCODE_MOTION_CCW_SYMBOL) == 0) {
    op->opcode = copyNewString(symbol);
  }
  else if (strcmp(symbol, OPCODE_DWELL_SYMBOL) == 0) {
    op->opcode = copyNewString(symbol);
  }
  else if (strcmp(symbol, OPCODE_HEADER_SYMBOL) == 0) {

    if (context->saw_header != 0) {
      printf("header opcode can only appear once in a file, line %d\n", context->lineNumber);
      return EBADF;
    }

    op->opcode = copyNewString(symbol);

    // Mark that header is seen
    context->saw_header = 1;
  }
  else if (strcmp(symbol, OPCODE_CONFIG_SYMBOL) == 0) {

    if (context->saw_config != 0) {
      printf("config opcode can only appear once in a file, line %d\n", context->lineNumber);
      return EBADF;
    }

    op->opcode = copyNewString(symbol);

    // Mark that config is seen
    context->saw_config = 1;
  }
  else {
    printf("unrecognized opcode %s\n", symbol);
    return EBADF;
  }

  return 0;
}

void
initialize_axis_opcode(PAXIS_OPCODE o)
{

  //
  // Strings could have been allocated in an assembler pass.
  //
  if (o->axis != NULL) {
    free(o->axis);
  }

  o->axis = NULL;

  if (o->opcode != NULL) {
    free(o->opcode);
  }

  o->opcode = NULL;

  if (o->arg0 != NULL) {
    free(o->arg0);
  }

  o->arg0 = NULL;

  if (o->arg1 != NULL) {
    free(o->arg1);
  }

  o->arg1 = NULL;

  if (o->arg2 != NULL) {
    free(o->arg2);
  }

  o->arg2 = NULL;
}

void
initialize_opcode_block_four_axis(POPCODE_BLOCK_FOUR_AXIS b)
{
  initialize_axis_opcode(&b->info);
  initialize_axis_opcode(&b->x);
  initialize_axis_opcode(&b->y);
  initialize_axis_opcode(&b->z);
  initialize_axis_opcode(&b->a);
}

void
initialize_axis_opcode_binary(PAXIS_OPCODE_BINARY o)
{
  o->instruction = OPCODE_NOP;
  o->pulse_rate = 0;
  o->pulse_count = 0;
  o->pulse_width = 0;
}

//
// Initialize all fields to begin a new assembly fine.
//
void
initialize_assembler_context(PASSEMBLER_CONTEXT context)
{
  bzero((void*)context, sizeof(ASSEMBLER_CONTEXT));

  context->lineNumber = 0;

  reset_assembler_context(context);
}

//
// Reset the current assembler block.
//
void
reset_assembler_context(PASSEMBLER_CONTEXT context)
{

  reset_current_assembler_opcode(context);

  reset_current_assembler_axis(context);

  initialize_opcode_block_four_axis(&context->opcode_block);

  context->opcode_block_valid = 0;

  context->saw_begin = 0;

  context->saw_end = 0;

  context->single_line_block = 0;

  context->x_set = 0;
  context->y_set = 0;
  context->z_set = 0;

  context->a_set = 0;
  context->b_set = 0;
  context->c_set = 0;

  context->u_set = 0;
  context->v_set = 0;
  context->w_set = 0;
}

//
// Reset the current axis block
//
void
reset_current_assembler_axis(PASSEMBLER_CONTEXT context)
{
  context->axis_state = AxisStateIdle;

  //
  // Note: This is a pointer to a containing axis structure
  // so its contents are not freed as the container structures
  // resources are freed separately.
  //
  context->current_axis_opcode = NULL;

  if (context->current_axis_symbol != NULL) {
    freeString(context->current_axis_symbol);
  }

  context->current_axis_symbol = NULL;
}

//
// Reset the current opcode block
//
void
reset_current_assembler_opcode(PASSEMBLER_CONTEXT context)
{
  context->opcode_state = OpCodeStateIdle;

  initialize_axis_opcode_binary(&context->opcode_binary);
}

void
debug_print_string_or_null(char* s)
{
#if DBG_PRINT_ENABLED
  if (s != NULL) {
    printf("%s\n", s);
  }
  else {
    printf("NULL\n");
  }
#endif    
}

void
dump_axis_opcode(PASSEMBLER_CONTEXT context, PAXIS_OPCODE opcode)
{
  debug_print_string_or_null(opcode->axis);
  debug_print_string_or_null(opcode->opcode);
  debug_print_string_or_null(opcode->arg0);
  debug_print_string_or_null(opcode->arg1);
  debug_print_string_or_null(opcode->arg2);
}

void
dump_opcode_block_four_axis(PASSEMBLER_CONTEXT context, POPCODE_BLOCK_FOUR_AXIS b)
{
  dump_axis_opcode(context, &b->info);
  dump_axis_opcode(context, &b->x);
  dump_axis_opcode(context, &b->y);
  dump_axis_opcode(context, &b->z);
  dump_axis_opcode(context, &b->a);
}

int
process_assembly_block(PASSEMBLER_CONTEXT context)
{
  int ret;
  OPCODE_BLOCK_FOUR_AXIS_BINARY bin;
  void* newBin;

  bzero(&bin, sizeof(bin));

  DBG_PRINT1("    process_assembly_block Line: %d\n", context->lineNumber);

  dump_opcode_block_four_axis(context, &context->opcode_block);

  ret = compile_assembly_block(context, &context->opcode_block, &bin);
  if (ret != 0) {
    return ret;
  }

  //
  // Push the compiled block of binary instructions
  // across the axis into the dynamic block array.
  //

  newBin = block_array_push_entry(
               context->compiled_binary,
	       &bin,
	       sizeof(bin)
	       );

  if (newBin == NULL) {
    return ENOMEM;
  }

  return 0;
}

//
// Compiled the symbolic opcode block to binary.
//
// This compiles across four axis, plus the virtual
// resource axis INFO.
//
int
compile_assembly_block(
    PASSEMBLER_CONTEXT context,
    POPCODE_BLOCK_FOUR_AXIS symbolic,
    POPCODE_BLOCK_FOUR_AXIS_BINARY bin
    )
{
  int ret;

  // TODO: How to handle info? begin/end?
  // dump_axis_opcode(context, &b->info);

  ret = compile_axis_opcode(context, &symbolic->x, &bin->x);
  if (ret != 0) {
    return ret;
  }

  ret = compile_axis_opcode(context, &symbolic->y, &bin->y);
  if (ret != 0) {
    return ret;
  }

  ret = compile_axis_opcode(context, &symbolic->z, &bin->z);
  if (ret != 0) {
    return ret;
  }

  ret = compile_axis_opcode(context, &symbolic->a, &bin->a);
  if (ret != 0) {
    return ret;
  }

  return 0;
}

int
compile_axis_opcode(
    PASSEMBLER_CONTEXT context,
    PAXIS_OPCODE symbolic,
    PAXIS_OPCODE_BINARY bin
    )
{
  int ret;

  //
  // symbolic->axis is informative, not used since instructions
  // are placed into the binary execution block by position.
  //

  DBG_PRINT2("compile_axis_opcode axis %s, line %d\n", symbolic->axis, context->lineNumber);

  //
  // Note entries may be NULL and NOP's are placed in the compiled opcode.
  //

  bin->instruction = opcode_symbol_to_binary(symbolic->opcode);
  if (bin->instruction == (-1)) {
    return ERANGE;
  }

  ret = string_to_unsigned_long(symbolic->arg0, &bin->pulse_rate);
  if (ret != 0) {
    return ret;
  }

  ret = string_to_unsigned_long(symbolic->arg1, &bin->pulse_count);
  if (ret != 0) {
    return ret;
  }

  ret = string_to_unsigned_long(symbolic->arg2, &bin->pulse_width);
  if (ret != 0) {
    return ret;
  }

  return 0;  
}

//
// Convert a string to an unsigned long.
//
// format:
//
// 123 - decimal number
// 0x123 - hexadecimal number
//
// Return 0 if NULL.
//
int
string_to_unsigned_long(char* s, unsigned long* l)
{
  long value;
  char* endptr;

  if (s == NULL) {
    *l = 0;
    return 0;
  }

  endptr = NULL;

  //
  // Base 0 automatically handles decimal, hex is 0x, or octal if 0.
  //

  errno = 0; // To distinguish success/failure after call

  value = strtol(s, &endptr, 0);

    // Check for various possible errors
  if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
        || (errno != 0 && value == 0)) {

    return errno;
  }

  if (s == endptr) {
    // No digits were found
    return ERANGE;
  }

  if (*endptr != '\0') {
    // endptr points to first character not converted.
    return ERANGE;
  }

  if (value < 0) {
    // Only unsigned is allowed
    return ERANGE;
  }

  // Success, all characters were numbers.
  *l = (unsigned long)value;

  return 0;
}

int
opcode_symbol_to_binary(char* s)
{
  // if null, the opcode is NOP
  if (s == NULL) {
    return OPCODE_NOP;
  }

  if (strcmp(s, OPCODE_NOP_SYMBOL) == 0) {
    return OPCODE_NOP;
  }
  else if (strcmp(s, OPCODE_MOTION_CW_SYMBOL) == 0) {
    return OPCODE_MOTION_CW;
  }
  else if (strcmp(s, OPCODE_MOTION_CCW_SYMBOL) == 0) {
    return OPCODE_MOTION_CCW;
  }
  else if (strcmp(s, OPCODE_DWELL_SYMBOL) == 0) {
    return OPCODE_DWELL;
  }
  else if (strcmp(s, OPCODE_HEADER_SYMBOL) == 0) {
    return OPCODE_HEADER;
  }
  else if (strcmp(s, OPCODE_CONFIG_SYMBOL) == 0) {
    return OPCODE_CONFIG;
  }
  else {
    return -1;
  }
}

int
axis_symbol_to_binary(char* s)
{
  if (strcmp(s, X_AXIS_SYMBOL) == 0) {
    return AXIS_CODE_X;
  }
  else if (strcmp(s, Y_AXIS_SYMBOL) == 0) {
    return AXIS_CODE_Y;
  }
  else if (strcmp(s, Z_AXIS_SYMBOL) == 0) {
    return AXIS_CODE_Z;
  }
  else if (strcmp(s, A_AXIS_SYMBOL) == 0) {
    return AXIS_CODE_A;
  }
  else if (strcmp(s, INFO_AXIS_SYMBOL) == 0) {
    return AXIS_CODE_INFO;
  }
  else {
    return -1;
  }
}

//
// Block array support for holding compiled instructions.
//
// Make it a realloc of a packet array for linear memory layout to be
// used to place instructions blocks into the machine pipeline.
//
// A symbol focused one would use a dynamic array of pointers
// but in this case we are optimizing for streams.
//

PBLOCK_ARRAY
block_array_allocate(int entry_size, int array_size, int array_increment_size)
{
  PBLOCK_ARRAY ba;
  int total_length;

  ba = (PBLOCK_ARRAY)malloc(sizeof(BLOCK_ARRAY));
  if (ba == NULL) {
    return (PBLOCK_ARRAY)NULL;
  }

  bzero((void*)ba, sizeof(BLOCK_ARRAY));

  total_length = entry_size * array_size;

  ba->blocks = malloc(total_length);
  if (ba->blocks == NULL) {
    free(ba);
    return (PBLOCK_ARRAY)NULL;
  }

  bzero(ba->blocks, total_length);

  ba->entry_size = entry_size;

  ba->array_capacity = array_size;

  ba->array_increment_size = array_increment_size;

  // Starts with zero entries
  ba->array_size = 0;

  ba->seek_index = 0;

  return ba;
}

void
block_array_free(PBLOCK_ARRAY ba)
{
  free(ba->blocks);
  free(ba);
  return;
}

void*
block_array_get_entry(PBLOCK_ARRAY ba, int index)
{
  char* tmp;

  if (index >= ba->array_size) {
    return NULL;
  }

  tmp = (char*)ba->blocks;
  tmp += (index * ba->entry_size);

  return (void*)tmp;
}

//
// entry_size is a check. If you try and push a size not specified
// when the block array is created it causes a -1 return.
//
// Returns the newly allocated entry index.
//
void*
block_array_push_entry(PBLOCK_ARRAY ba, void* entry, int entry_size)
{
  char* tmp;
  void* new_blocks;
  int new_capacity;

  if (entry_size != ba->entry_size) {
    return (void*)NULL;
  }

  if (ba->array_size >= (ba->array_capacity - 1)) {
    // realloc
    new_capacity = ba->array_capacity + ba->array_increment_size;

    new_blocks = realloc(ba->blocks, new_capacity);
    if (new_blocks == NULL) {
      return (void*)NULL;
    }

    // realloc has copied the memory, but not initialized the new bytes
    tmp = (char*)new_blocks;

    // Advance to end of current capacity
    tmp += (ba->entry_size * ba->array_capacity);

    bzero(tmp, new_capacity - (ba->array_size * ba->array_capacity));

    ba->blocks = new_blocks;
    ba->array_capacity = new_capacity;    
  }

  // Advance to next entry
  ba->array_size++;

  tmp = (char*)ba->blocks;
  tmp += (ba->array_size * ba->entry_size);

  bcopy(tmp, entry, entry_size);

  return (void*)tmp;
}

int
block_array_get_array_size(PBLOCK_ARRAY ba)
{
  return ba->array_size;
}

//
// Open and load the specified assembly file, assemble it
// and return the binary instructions in memory.
//
int
assemble_file(char* fileName, PBLOCK_ARRAY* binary)
{
  int ret;
  FILE *file;
  PASSEMBLER_CONTEXT context = NULL;

  file = fopen(fileName, "r");
  if (file == NULL) {
    DBG_PRINT2("error opening file errno %d file %s\n", errno, fileName);
    return errno;
  }

  // Allocate context
  context = (PASSEMBLER_CONTEXT)malloc(sizeof(ASSEMBLER_CONTEXT));
  bzero((void*)context, sizeof(ASSEMBLER_CONTEXT));

  // Begin a new assembly file/program
  initialize_assembler_context(context);

  // Allocate block array
  context->compiled_binary = block_array_allocate(
      sizeof(OPCODE_BLOCK_FOUR_AXIS_BINARY),
      BLOCK_ARRAY_INITIAL_ALLOCATION,
      BLOCK_ARRAY_INCREMENTAL_ALLOCATION
      );

  ret = processLines(context, file);

  fclose(file);

  if (ret != 0) {
    DBG_PRINT3("assembler error lineno %d, (%d) %s, exiting\n",
           context->lineNumber, ret, strerror(ret));

    return ret;
  }

  DBG_PRINT1("assembled %d opcode blocks\n", block_array_get_array_size(context->compiled_binary));

  *binary = context->compiled_binary;

  return 0;
}

//
// Seek stream in block array
//
// Seeks to a specific index.
//
// Seeking to 0 is rewind.
//
int
block_array_seek_entry(PBLOCK_ARRAY ba, int entry_index)
{
  if (ba->seek_index > ba->array_size) {
      return EBADF;
  }

  ba->seek_index = entry_index;

  return 0;
}

//
// Get the next entry.
//
// Returns NULL if not more entries.
//
// Intended for high speed streaming of binary commands to
// a hardware interface with low per entry processor cycle overheads.
//
void*
block_array_get_next_entry(PBLOCK_ARRAY ba)
{
  void* block;

  //
  // TODO: Do a more efficient pointer bump for weak
  // embedded processors operating in real time.
  //

  block = block_array_get_entry(ba, ba->seek_index);

  if (block != NULL) {
    ba->seek_index++;
  }

  return block;
}

//
// Process assembler lines from a file stream.
//
int
processLines(
    PASSEMBLER_CONTEXT context,
    FILE* f
    )
{
  int ret;
  ssize_t read;
  size_t len;
  char *line;

  //
  // Setting lineptr to NULL has getline allocate the string memory
  //
  // It will re-allocate it as needed in the loop.
  //
  line = NULL;
  len = 0;

  while ((read = getline(&line, &len, f)) != -1) {
    ret = process_assembler_line(context, line);
    if (ret != 0) {
      return ret;
    }
    context->lineNumber++;
  }

  free(line);

  return 0;
}
