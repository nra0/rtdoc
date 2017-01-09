#include "../lib.h"
#include "testDoc.h"
#include "../../src/doc.h"
#include "../../src/mmalloc.h"

#include <stdio.h>


Document *doc;
Collaborator *user;
Json *contents;
char *err;


static void setup(void) {
  err = mmalloc(128);
}

static void teardown(void) {
  mfree(err);
  assertEqual(0, memoryUsage());
}


static void testDocumentGetInfo(void) {
  char *contentString = "[1,2,3]", *key = "key";
  contents = jsonParse(contentString, &err);
  doc = documentCreate(key, contents);
  assertPointerEqual(contents, documentGetContents(doc));
  assertEqual(0, listLength(documentGetCollaborators(doc)));
  documentFree(doc);
}

static void testCollaboratorGetInfo(void) {
  char *userId = "0123";
  user = collaboratorCreate(userId);
  assertStringEqual(userId, collaboratorGetKey(user));
  collaboratorFree(user);
}

static void testDocumentAddCollaborators(void) {
  doc = documentCreate("key", jsonParse("{}", &err));
  int numUsers = 128;
  char key[128];
  for (int i = 0; i < numUsers; i++) {
    sprintf(key, "key%d", i);
    documentAddCollaborator(doc, collaboratorCreate(key));
    assertEqual(i + 1, listLength(documentGetCollaborators(doc)));
  }
  documentFree(doc);
}

static void testDocumentRemoveCollaborators(void) {
  doc = documentCreate("key", jsonParse("{}", &err));
  int numUsers = 128;
  char key[128];
  for (int i = 0; i < numUsers; i++) {
    sprintf(key, "key%d", i);
    documentAddCollaborator(doc, collaboratorCreate(key));
  }
  for (int i = 0; i < numUsers; i++) {
    sprintf(key, "key%d", i);
    documentRemoveCollaborator(doc, key);
    assertEqual(numUsers - i - 1, listLength(documentGetCollaborators(doc)));
  }
  documentFree(doc);
}


TestSuite *documentTestSuite() {
  TestSuite *suite = testSuiteCreate("Document and collaborators", &setup, &teardown);
  testSuiteAdd(suite, "doc get info", &testDocumentGetInfo);
  testSuiteAdd(suite, "collaborator get info", &testCollaboratorGetInfo);
  testSuiteAdd(suite, "add collaborators", &testDocumentAddCollaborators);
  testSuiteAdd(suite, "remove collaborators", &testDocumentRemoveCollaborators);
  return suite;
}
