#ifndef _EXCEPTION_HANDLING__H_
#define _EXCEPTION_HANDLING__H_


//uncomment this line if you want Exceptions to be handled
#ifdef JA2

#ifndef _DEBUG
	#define ENABLE_EXCEPTION_HANDLING
#endif

#else
	//Wizardry
//#define ENABLE_EXCEPTION_HANDLING
#endif


#ifdef __cplusplus
extern "C" {
#endif



INT32 RecordExceptionInfo( EXCEPTION_POINTERS *pExceptInfo );


#ifdef __cplusplus
}
#endif

#endif
