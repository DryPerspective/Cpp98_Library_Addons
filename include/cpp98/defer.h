#ifndef DP_CPP98_DEFER
#define DP_CPP98_DEFER

/*
*  A DEFER tool, also known as a SCOPE_EXIT tool. If a function should contain the expression DEFER(foo) then foo will automatically be executed when the scope exits.
*  Note that due to the limitations of C++98, this tool is not aware of any function-local data and can only call stateless code or code which manipulates some global state (ugh).
*  Still, good for when using what is essentially a C library as you can automate cleanup.
*/
#define DEFER_CONCAT_IMPL(x,y) x##y
#define DEFER_CONCAT_MACRO( x, y ) DEFER_CONCAT_IMPL( x, y )

#ifdef __COUNTER__
#define DEFER_CONCAT_COUNT __COUNTER__
#else
#define DEFER_CONCAT_COUNT __LINE__
#endif

#define DEFER(ARGS) struct { \
                        struct Defer_Impl{ \
                            Defer_Impl() {}; \
                            ~Defer_Impl() { ARGS ;} \
                        }  impl; \
                    } DEFER_CONCAT_MACRO(Defer_Struct, DEFER_CONCAT_COUNT);


#endif