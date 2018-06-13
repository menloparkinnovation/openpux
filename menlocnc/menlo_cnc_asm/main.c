
//
// main.c, menlo_cnc_asm - Assembler for menlo_cnc, FPGA based machine tool controller.
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

#include <string.h> // strtok, bzero

#include "menlo_cnc_asm.h"

void usage();

//
// Standalone front end driver for menlo_cnc_asm.c library module.
//

int
main(int ac, char* av[])
{
  int ret;
  char* fileName;
  PBLOCK_ARRAY binary = NULL;
  
  if (ac < 2) {
    usage();
  }

  fileName = av[1];

  ret = assemble_file(fileName, &binary);

  if (ret != 0) {
    printf("assembler error %d %s, exiting\n", ret, strerror(ret));
    return ret;
  }

  printf("assembled %d opcode blocks\n", block_array_get_array_size(binary));
  printf("assembly success, exiting\n");

  //
  // context->compiled_binary is a pointer to the block array
  // that contains the compiled assembly instructions as machine
  // code that can be saved, fed to the registers, placed into memory
  // for DMA, etc.
  //

  return 0;
}

void
usage()
{
  fprintf(stderr, "usage: menlo_cnc_asm filename.txt\n");
  exit(1);
}
