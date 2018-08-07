#ifndef __PAULA_HDR_CLIPARAMETERS__
#define __PAULA_HDR_CLIPARAMETERS__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CLIParameters
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLIParameters {
  private:
    int          argCount;
    const char** argList;

  public:
    CLIParameters(int argCount, const char** argList) : argCount(argCount - 1), argList(argList + 1) { }

    /**
     * Returns true if the CLI parameters contain the named argument
     */
    bool has(const char* name) const;

    /**
     * Returns the string value associated with the named argument, or the provided default if no value
     * is assoicated or if the argument was not provided.
     */
    const char* get(const char* name, const char* def = 0) const ;
};


#endif
