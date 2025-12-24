/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Logo_svg;
    const int            Logo_svgSize = 112529;

    extern const char*   backgroundGradient_png;
    const int            backgroundGradient_pngSize = 194289;

    extern const char*   LatoBlack_ttf;
    const int            LatoBlack_ttfSize = 69500;

    extern const char*   PHONES_TTF;
    const int            PHONES_TTFSize = 35004;

    extern const char*   FrankRuehlCLM_ttf;
    const int            FrankRuehlCLM_ttfSize = 43904;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
