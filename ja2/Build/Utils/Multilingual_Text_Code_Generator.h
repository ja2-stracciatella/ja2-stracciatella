#ifdef _DEBUG

//If special command line argument is used, then the utility will kick in
//and activate the special code generator.
BOOLEAN ProcessIfMultilingualCmdLineArgDetected( UINT8 *str );

#else

//macro function out
#define ProcessIfMultilingualCmdLineArgDetected( a )	0

#endif
