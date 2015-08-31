/* Copyright (c) 2015, codestoke
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of psprint nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>

#include <windows.h>

using namespace std;

int rawPrint(LPTSTR printerName, LPBYTE data, DWORD count, int tray) {
    HANDLE hPrinter;
    DOC_INFO_1 docInfo;
    DWORD job;
    DWORD bytesWritten;

    if (!OpenPrinter(printerName, &hPrinter, nullptr)) {
        cerr << GetLastError() << ": could not open printer.\n";
        return -1;
    }

    docInfo.pDocName = (LPSTR) ("TEST TRAY " + std::to_string(tray)).c_str();
    docInfo.pOutputFile = nullptr;
    docInfo.pDatatype = (LPSTR) "RAW";

    job = StartDocPrinter(hPrinter, 1, (LPBYTE) &docInfo);

    if (job == 0) {
        cerr << GetLastError() << ": could not start doc printer\n";
        ClosePrinter(hPrinter);
        return -1;
    }

    if (!StartPagePrinter(hPrinter)) {
        cerr << GetLastError() << ": could not start page printer\n";
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        return -1;
    }

    if (!WritePrinter(hPrinter, data, count, &bytesWritten)) {
        cerr << GetLastError() << ": could not send data to printer\n";
        EndPagePrinter(hPrinter);
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        return -1;
    }

    if (!EndPagePrinter(hPrinter)) {
        cerr << GetLastError() << ": could not end page printer\n";
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        return -1;
    }

    if (!EndDocPrinter(hPrinter)) {
        cerr << GetLastError() << ": could not end doc printer\n";
        ClosePrinter(hPrinter);
        return -1;
    }

    ClosePrinter(hPrinter);

    return (int) bytesWritten;
}

string generateData(int tray, string username) {
    string postscript = "\x1b"
                                "%-12345X@PJL JOB\n"
                                "@PJL SET USERNAME=\"" + username + "\"\n"
                                "@PJL SET USERNAMEW=\"" + username + "\"\n"
                                "\x1b"
                                "%-12345X@PJL ENTER LANGUAGE=POSTSCRIPT\n"
                                "%!PS-Adobe-3.0\n"
                                "%%LanguageLevel: 3\n"
                                "%%Pages: 1\n"
                                "%%BoundingBox: 0 0 596 842\n"
                                "%%HiResBoundingBox: 0 0 595.276 841.89\n"
                                "%%EndComments\n"
                                "%%BeginDefaults\n"
                                "%%EndDefaults\n"
                                "%%BeginProlog\n"
                                "%%EndProlog\n"
                                "%%BeginSetup\n"
                                "%%EndSetup\n"
                                "%%Page: 1 1\n"
                                "%%PageBoundingBox: 0 0 595 842\n"
                                "%%PageHiResBoundingBox: 0 0 595.276 841.89\n"
                                "%%BeginPageSetup\n"
                                "<<\n"
                                "/MediaPosition " + std::to_string(tray) + "\n"
                                "/PageSize [595 842]\n"
                                ">> setpagedevice\n"
                                "%%EndPageSetup\n"
                                "/Times-Roman findfont\n"
                                "32 scalefont\n"
                                "setfont\n"
                                "72 770 moveto\n"
                                "(MediaPosition " + std::to_string(tray) + ") show\n"
                                "showpage\n"
                                "%%EOF\n"
                                "\x04\x1b"
                                "%-12345X@PJL EOJ\n"
                                "\x1b"
                                "%-12345X";

    return postscript;
}

void print(string username, int tray, string printer) {
    string data = generateData(tray, username);
    rawPrint((LPTSTR) printer.c_str(), (LPBYTE) data.c_str(), (DWORD) data.length(), tray);
}

int main(int argc, char **argv) {
    cout << "test a printer and its trays." << endl;

    print("xyz", 3, "\\\\server\\printer");

    return 0;
}