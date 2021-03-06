[< Back](.)

io
================

The `io` module provides an interface to the host machine's file system, in order to read and write data files.

It contains the following classes:

* [FileSystem](#filesystem)

## FileSystem

### Static Methods

#### `static load(path: String): String`
Given a valid file `path`, this loads the file data into a String object.
This is a blocking operation, and so execution will stop while the file is loaded.

#### `static save(path: String, buffer: String): Void`
Given a valid file `path`, this will create or overwrite the file the data in the `buffer` String object.
This is a blocking operation, and so execution will stop while the file is saved.
