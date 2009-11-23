#define C_LUCY_RAMFOLDER
#define C_LUCY_CHARBUF
#include "Lucy/Util/ToolSet.h"

#define CHAZ_USE_SHORT_NAMES
#include "Charmonizer/Test.h"

#include "Lucy/Test/Store/TestRAMFolder.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Store/RAMFileHandle.h"

static CharBuf foo           = ZCB_LITERAL("foo");
static CharBuf bar           = ZCB_LITERAL("bar");
static CharBuf baz           = ZCB_LITERAL("baz");
static CharBuf boffo         = ZCB_LITERAL("boffo");
static CharBuf banana        = ZCB_LITERAL("banana");
static CharBuf foo_bar       = ZCB_LITERAL("foo/bar");
static CharBuf foo_bar_baz   = ZCB_LITERAL("foo/bar/baz");
static CharBuf foo_bar_boffo = ZCB_LITERAL("foo/bar/boffo");
static CharBuf foo_boffo     = ZCB_LITERAL("foo/boffo");
static CharBuf foo_foo       = ZCB_LITERAL("foo/foo");
static CharBuf nope          = ZCB_LITERAL("nope");
static CharBuf nope_nyet     = ZCB_LITERAL("nope/nyet");

static void
test_Initialize_and_Check(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    Folder_Initialize(folder);
    PASS(batch, "Initialized concludes without incident");
    ASSERT_TRUE(batch, Folder_Check(folder), "Check succeeds");
    DECREF(folder);
}

static void
test_Local_Exists(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh = Folder_Open_FileHandle(folder, &boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    Folder_Local_MkDir(folder, &foo);

    ASSERT_TRUE(batch, Folder_Local_Exists(folder, &boffo), 
        "Local_Exists() returns true for file");
    ASSERT_TRUE(batch, Folder_Local_Exists(folder, &foo), 
        "Local_Exists() returns true for dir");
    ASSERT_FALSE(batch, Folder_Local_Exists(folder, &bar), 
        "Local_Exists() returns false for non-existent entry");

    DECREF(folder);
}

static void
test_Local_Is_Directory(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh = Folder_Open_FileHandle(folder, &boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    Folder_Local_MkDir(folder, &foo);

    ASSERT_FALSE(batch, Folder_Local_Is_Directory(folder, &boffo), 
        "Local_Is_Directory() returns false for file");
    ASSERT_TRUE(batch, Folder_Local_Is_Directory(folder, &foo), 
        "Local_Is_Directory() returns true for dir");
    ASSERT_FALSE(batch, Folder_Local_Is_Directory(folder, &bar), 
        "Local_Is_Directory() returns false for non-existent entry");

    DECREF(folder);
}

static void
test_Local_Find_Folder(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    RAMFolder *local;
    FileHandle *fh;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    fh = Folder_Open_FileHandle(folder, &boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, &foo_boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, &nope);
    ASSERT_TRUE(batch, local == NULL, "Non-existent entry yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, (CharBuf*)&EMPTY);
    ASSERT_TRUE(batch, local == NULL, "Empty string yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, &foo_bar);
    ASSERT_TRUE(batch, local == NULL, "nested folder yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, &foo_boffo);
    ASSERT_TRUE(batch, local == NULL, "nested file yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, &boffo);
    ASSERT_TRUE(batch, local == NULL, "local file yields NULL");
    
    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, &bar);
    ASSERT_TRUE(batch, local == NULL, "name of nested folder yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, &foo);
    ASSERT_TRUE(batch, 
        local 
        && Obj_Is_A(local, RAMFOLDER)
        && CB_Equals_Str(RAMFolder_Get_Path(local), "foo", 3), 
        "Find local directory");

    DECREF(folder);
}

static void
test_Local_MkDir(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    bool_t result;

    result = Folder_Local_MkDir(folder, &foo);
    ASSERT_TRUE(batch, result, "Local_MkDir succeeds and returns true");

    Err_set_error(NULL);
    result = Folder_Local_MkDir(folder, &foo);
    ASSERT_FALSE(batch, result, 
        "Local_MkDir returns false when a dir already exists");
    ASSERT_TRUE(batch, Err_get_error() != NULL, 
        "Local_MkDir sets Err_error when a dir already exists");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo), 
        "Existing dir untouched after failed Local_MkDir");

    {
        FileHandle *fh = Folder_Open_FileHandle(folder, &boffo, 
            FH_CREATE | FH_WRITE_ONLY);
        DECREF(fh);
        Err_set_error(NULL);
        result = Folder_Local_MkDir(folder, &foo);
        ASSERT_FALSE(batch, result, 
            "Local_MkDir returns false when a file already exists");
        ASSERT_TRUE(batch, Err_get_error() != NULL, 
            "Local_MkDir sets Err_error when a file already exists");
        ASSERT_TRUE(batch, Folder_Exists(folder, &boffo) &&
            !Folder_Local_Is_Directory(folder, &boffo), 
            "Existing file untouched after failed Local_MkDir");
    }

    DECREF(folder);
}

static void
test_Local_Open_FileHandle(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;

    fh = Folder_Local_Open_FileHandle(folder, &boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    ASSERT_TRUE(batch, fh && Obj_Is_A(fh, RAMFILEHANDLE), 
        "opened FileHandle");
    DECREF(fh);

    fh = Folder_Local_Open_FileHandle(folder, &boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    ASSERT_TRUE(batch, fh && Obj_Is_A(fh, RAMFILEHANDLE), 
        "opened FileHandle for append");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Local_Open_FileHandle(folder, &boffo, 
        FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    ASSERT_TRUE(batch, fh == NULL, "FH_EXLUSIVE flag prevents open");
    ASSERT_TRUE(batch, Err_get_error() != NULL,
        "failure due to FH_EXLUSIVE flag sets Err_error");

    fh = Folder_Local_Open_FileHandle(folder, &boffo, FH_READ_ONLY);
    ASSERT_TRUE(batch, fh && Obj_Is_A(fh, RAMFILEHANDLE), 
        "opened FileHandle for reading");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Local_Open_FileHandle(folder, &nope, FH_READ_ONLY);
    ASSERT_TRUE(batch, fh == NULL, 
        "Can't open non-existent file for reading");
    ASSERT_TRUE(batch, Err_get_error() != NULL,
        "Opening non-existent file for reading sets Err_error");

    DECREF(folder);
}

static void
test_Local_Delete(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;
    
    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    ASSERT_TRUE(batch, Folder_Local_Delete(folder, &boffo), 
        "Local_Delete on file succeeds");

    Folder_Local_MkDir(folder, &foo);
    fh = Folder_Open_FileHandle(folder, &foo_boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    Err_set_error(NULL);
    ASSERT_FALSE(batch, Folder_Local_Delete(folder, &foo), 
        "Local_Delete on non-empty dir fails");

    Folder_Delete(folder, &foo_boffo);
    ASSERT_TRUE(batch, Folder_Local_Delete(folder, &foo), 
        "Local_Delete on empty dir succeeds");

    DECREF(folder);
}

static void
test_Rename(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;
    bool_t result;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    /* Move files. */

    result = Folder_Rename(folder, &boffo, &banana); 
    ASSERT_TRUE(batch, result, "Rename succeeds and returns true");
    ASSERT_TRUE(batch, Folder_Exists(folder, &banana), 
        "File exists at new path");
    ASSERT_FALSE(batch, Folder_Exists(folder, &boffo), 
        "File no longer exists at old path");

    result = Folder_Rename(folder, &banana, &foo_bar_boffo); 
    ASSERT_TRUE(batch, result, "Rename to file in nested dir");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_bar_boffo), 
        "File exists at new path");
    ASSERT_FALSE(batch, Folder_Exists(folder, &banana), 
        "File no longer exists at old path");

    result = Folder_Rename(folder, &foo_bar_boffo, &boffo); 
    ASSERT_TRUE(batch, result, "Rename from file in nested dir");
    ASSERT_TRUE(batch, Folder_Exists(folder, &boffo), 
        "File exists at new path");
    ASSERT_FALSE(batch, Folder_Exists(folder, &foo_bar_boffo), 
        "File no longer exists at old path");

    fh = Folder_Open_FileHandle(folder, &foo_boffo, 
        FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    result = Folder_Rename(folder, &boffo, &foo_boffo); 
    ASSERT_TRUE(batch, result, "Clobber");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File exists at new path");
    ASSERT_FALSE(batch, Folder_Exists(folder, &boffo), 
        "File no longer exists at old path");

    /* Move Dirs. */

    Folder_MkDir(folder, &baz);
    result = Folder_Rename(folder, &baz, &boffo); 
    ASSERT_TRUE(batch, result, "Rename dir");
    ASSERT_TRUE(batch, Folder_Exists(folder, &boffo), 
        "Folder exists at new path");
    ASSERT_FALSE(batch, Folder_Exists(folder, &baz), 
        "Folder no longer exists at old path");

    result = Folder_Rename(folder, &boffo, &foo_foo); 
    ASSERT_TRUE(batch, result, "Rename dir into nested subdir");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_foo), 
        "Folder exists at new path");
    ASSERT_FALSE(batch, Folder_Exists(folder, &boffo), 
        "Folder no longer exists at old path");

    result = Folder_Rename(folder, &foo_foo, &foo_bar_baz); 
    ASSERT_TRUE(batch, result, "Rename dir from nested subdir");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_bar_baz), 
        "Folder exists at new path");
    ASSERT_FALSE(batch, Folder_Exists(folder, &foo_foo), 
        "Folder no longer exists at old path");
    
    /* Test failed clobbers. */

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_boffo, &foo_bar); 
    ASSERT_FALSE(batch, result, "Rename file clobbering dir fails");
    ASSERT_TRUE(batch, Err_get_error() != NULL, 
        "Failed rename sets Err_error");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File still exists at old path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_bar), 
        "Dir still exists after failed clobber");

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_bar, &foo_boffo); 
    ASSERT_FALSE(batch, result, "Rename dir clobbering file fails");
    ASSERT_TRUE(batch, Err_get_error() != NULL, 
        "Failed rename sets Err_error");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_bar), 
        "Dir still exists at old path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File still exists after failed clobber");

    /* Test that "renaming" succeeds where to and from are the same. */

    result = Folder_Rename(folder, &foo_boffo, &foo_boffo); 
    ASSERT_TRUE(batch, result, "Renaming file to itself succeeds");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File still exists");

    result = Folder_Rename(folder, &foo_bar, &foo_bar); 
    ASSERT_TRUE(batch, result, "Renaming dir to itself succeeds");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_bar), 
        "Dir still exists");

    /* Invalid filepaths. */

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_boffo, &nope_nyet); 
    ASSERT_FALSE(batch, result, "Rename into non-existent subdir fails");
    ASSERT_TRUE(batch, Err_get_error() != NULL, 
        "Renaming into non-existent subdir sets Err_error");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "Entry still exists at old path");

    Err_set_error(NULL);
    result = Folder_Rename(folder, &nope_nyet, &boffo); 
    ASSERT_FALSE(batch, result, "Rename non-existent file fails");
    ASSERT_TRUE(batch, Err_get_error() != NULL, 
        "Renaming non-existent source file sets Err_error");

    DECREF(folder);
}

static void
test_Hard_Link(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;
    bool_t result;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    /* Link files. */

    result = Folder_Hard_Link(folder, &boffo, &banana); 
    ASSERT_TRUE(batch, result, "Hard_Link succeeds and returns true");
    ASSERT_TRUE(batch, Folder_Exists(folder, &banana), 
        "File exists at new path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &boffo), 
        "File still exists at old path");
    Folder_Delete(folder, &boffo);

    result = Folder_Hard_Link(folder, &banana, &foo_bar_boffo); 
    ASSERT_TRUE(batch, result, "Hard_Link to target within nested dir");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_bar_boffo), 
        "File exists at new path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &banana), 
        "File still exists at old path");
    Folder_Delete(folder, &banana);

    result = Folder_Hard_Link(folder, &foo_bar_boffo, &foo_boffo); 
    ASSERT_TRUE(batch, result, "Hard_Link from file in nested dir");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File exists at new path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_bar_boffo), 
        "File still exists at old path");
    Folder_Delete(folder, &foo_bar_boffo);

    /* Invalid clobbers. */

    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    result = Folder_Hard_Link(folder, &foo_boffo, &boffo); 
    ASSERT_FALSE(batch, result, "Clobber of file fails");
    ASSERT_TRUE(batch, Folder_Exists(folder, &boffo), 
        "File still exists at new path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File still exists at old path");
    Folder_Delete(folder, &boffo);

    Folder_MkDir(folder, &baz);
    result = Folder_Hard_Link(folder, &foo_boffo, &baz); 
    ASSERT_FALSE(batch, result, "Clobber of dir fails");
    ASSERT_TRUE(batch, Folder_Exists(folder, &baz), 
        "Dir still exists at new path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File still exists at old path");
    Folder_Delete(folder, &baz);

    /* Invalid Hard_Link of dir. */

    Folder_MkDir(folder, &baz);
    result = Folder_Hard_Link(folder, &baz, &banana); 
    ASSERT_FALSE(batch, result, "Hard_Link dir fails");
    ASSERT_FALSE(batch, Folder_Exists(folder, &banana), 
        "Nothing at new path");
    ASSERT_TRUE(batch, Folder_Exists(folder, &baz), 
        "Folder still exists at old path");
    Folder_Delete(folder, &baz);

    /* Test that linking to yourself fails. */

    result = Folder_Hard_Link(folder, &foo_boffo, &foo_boffo); 
    ASSERT_FALSE(batch, result, "Hard_Link file to itself fails");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "File still exists");

    /* Invalid filepaths. */

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_boffo, &nope_nyet); 
    ASSERT_FALSE(batch, result, "Hard_Link into non-existent subdir fails");
    ASSERT_TRUE(batch, Err_get_error() != NULL, 
        "Hard_Link into non-existent subdir sets Err_error");
    ASSERT_TRUE(batch, Folder_Exists(folder, &foo_boffo), 
        "Entry still exists at old path");

    Err_set_error(NULL);
    result = Folder_Rename(folder, &nope_nyet, &boffo); 
    ASSERT_FALSE(batch, result, "Hard_Link non-existent source file fails");
    ASSERT_TRUE(batch, Err_get_error() != NULL, 
        "Hard_Link non-existent source file sets Err_error");

    DECREF(folder);
}

static void
test_Close(TestBatch *batch)
{
    RAMFolder *folder = RAMFolder_new(NULL);
    RAMFolder_Close(folder);
    PASS(batch, "Close() concludes without incident");
    RAMFolder_Close(folder);
    RAMFolder_Close(folder);
    PASS(batch, "Calling Close() multiple times is safe");
    DECREF(folder);
}

void
TestRAMFolder_run_tests()
{
    TestBatch *batch = Test_new_batch("TestRAMFolder", 97, NULL);

    PLAN(batch);
    test_Initialize_and_Check(batch);
    test_Local_Exists(batch);
    test_Local_Is_Directory(batch);
    test_Local_Find_Folder(batch);
    test_Local_MkDir(batch);
    test_Local_Open_FileHandle(batch);
    test_Local_Delete(batch);
    test_Rename(batch);
    test_Hard_Link(batch);
    test_Close(batch);

    batch->destroy(batch);
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

