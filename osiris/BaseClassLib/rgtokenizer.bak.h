/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*  FileName: rgtokenizer.bak.h
*  Author:   Robert Goor
*
*/
//
// class RGStringTokenizer which extracts tokens from an RGString
//

#ifndef _RGSTRINGTOKENIZER_H_
#define _RGSTRINGTOKENIZER_H_

#include "rgdefs.h"
#include "persist.h"
#include "rgstring.h"
#include "rgdlist.h"

class RGStringArray;


class RGStringTokenizer {

public:
	RGStringTokenizer (RGString* str);
	~RGStringTokenizer ();

	void ClearTokens ();
	void AddDelimiter (const RGString& str);
	void AddDelimiter (const char* str);
	void AddDelimiter (char c);
	void AddDelimiters (RGDList& strlist);

	void ClearRemoveItems ();
	void AddRemoveItem (char c);

	Boolean FirstTokenIsNull (RGString*& delim);
	RGString* SkipLeadingDelimiter ();
	Boolean NextToken (RGString& Token, RGString& Delimiter);

	void ResetTokenizer ();
	void ResetString (RGString* NewStr);

	RGStringArray* Split (RGStringArray*& Delimiters);
	RGString* Join (const RGStringArray& s1, const RGStringArray& s2);

protected:
	RGString* Str;
	char* StringData;
	char* CurrentPtr;
	RGDList DelimiterList;  // list of RGStrings used for alternate delimiters
	RGDListIterator* it;    // iterator for DelimiterList

	RGDList RemoveList;   // list of RGStrings to be removed - each is a character
	RGDListIterator* itRemove;  // iterator for RemoveList

	Boolean CompareForDelimiterAtPointer (const RGString* delim, char* ptr);
	RGString* LookForAnyDelimiterAtPointer (char* ptr);
	Boolean CharacterIsInRemoveList (char c);
};



class RGStringArray {

public:
	RGStringArray ();    // default size
	RGStringArray (size_t size);   // specify size
	~RGStringArray ();

	RGString operator[](size_t) const;  // performs limit checking and expands array if needed
	RGString& operator[](size_t);      // performs limit checking and expands array if needed; deletes replaced item

	void Resize (size_t NewSize);
	void Append (const RGString& NewItem);
	void Append (const char* NewItem);

	void Replace (size_t index, const RGString& item);  // performs limit checking and expands array if needed
	void Replace (size_t index, const char* NewItem);
	int GetIntegerValue (size_t index) const;
	double GetDoubleValue (size_t index) const;
	size_t Length () const { return CurrentLength; }

	static void ResetDefaultSize (size_t NewSize);
	static void ResetDefaultIncrement (size_t Increment);

protected:
	static size_t DefaultSize;
	static size_t DefaultIncrement;
	static RGString NullString;

	size_t ArraySize;
	size_t CurrentLength;

	RGString** StringArray;

	size_t ComputeNewSize (size_t Target);
	size_t ComputeTargetSize (size_t Target);
	void NullNewItems (size_t First, size_t Last);
	void BuildNewArray (size_t size);
};


#endif  /*  _RGSTRINGTOKENIZER_H_  */
