
/*
 * Modifications, additions, subtractions are
 * Copyright (C) 2012 Menlo Park Innovation LLC
 *
 * This work incorporates one or more "open" or "shared" source
 * licenses. As such, its use and licensing conforms to the licenses
 * of the code incorporated.
 *
 * Menlo Park Innovation LLC does not provide any warranty, support,
 * or restrictions on its use and may be distributed under the
 * terms of the contained license(s).
 *
 *  Date: 11/20/2012
 *  File: OS_LibraryTemplate.h
 *
 * Code contained within comes from the following open, shared, community,
 * public domain, or other licenses:
 *
 */

#ifndef OS_LibraryTemplate_h
#define OS_LibraryTemplate_h

//
// Any inclusion of standard libraries is headers is "Library Use"
// licensing.
//
#include <Arduino.h>
#include <inttypes.h>

class OS_LibraryTemplate {

public:
  
  OS_LibraryTemplate();

  int Initialize();

private:

};

#endif // OS_LibraryTemplate_h
