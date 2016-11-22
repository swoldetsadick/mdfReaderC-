# mdfReaderC-
A Simple Hadoop ready MDFReader

This MDFSimpleReader is used as a solution for reading mdf files in a cluster using Spark.
It is based on the MDF4Reader COM element of http://www.turbolab.de/mdf_libf.htm to which small changes were introduced.

If you use the MDF4 C++ classes in your application you must be a member of ASAM e.V. or purchase the standard from ASAM e.V.. However, the use of the COM objects MDF4Writer and MDF4Reader is free and does not require ASAM membership.

### Tested on Linux
# Use
cd MDF4_SimpleReader/
make clean CONF=Release (or make clean)
make CONF=Release (or make)
./dist/Release/GNU-Linux/mdf4_simplereader ../Test.mf4
