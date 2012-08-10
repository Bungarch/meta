/**
 * @file porter2_stemmer.cpp
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "porter2_stemmer.h"

#define DEBUG 1

using namespace Porter2Stemmer::internal;
using std::string;
using std::vector;
using std::stringstream;
using std::endl;
using std::cout;

string Porter2Stemmer::stem(const string & toStem)
{
    if(DEBUG) cout << __func__ << ": " << toStem << endl;
    string word = trim(toStem);

    if(returnImmediately(word))
        return finalStem(word);

    if(special(word))
        return finalStem(word);

    changeY(word);
    int startR1 = getStartR1(word);
    int startR2 = getStartR2(word, startR1);
    removeApostrophe(word);

    if(step1A(word))
        return finalStem(word);
    
    step1B(word, startR1);
    step1C(word);
    step2(word, startR1);
    step3(word, startR1, startR2);
    step4(word, startR2);
    step5(word, startR1, startR2);

    return finalStem(word);
}

string Porter2Stemmer::internal::finalStem(string & word)
{
    std::replace(word.begin(), word.end(), 'Y', 'y');
    std::remove(word.begin(), word.end(), '\'');
    return word;
}

string Porter2Stemmer::trim(const string & toStem)
{
    string word = "";
    for(auto ch: toStem)
    {
        if(ch >= 'A' && ch <= 'Z')
            ch += 32;
        if((ch >= 'a' && ch <= 'z') || ch == '\'')
            word += ch;
    }

    // remove leading apostrophe
    if(word.length() >= 1 && word[0] == '\'')
        word = word.substr(1, word.length() - 1);

    return word;
}

bool Porter2Stemmer::internal::returnImmediately(const string & word)
{
    return word.length() <= 2;
}

int Porter2Stemmer::internal::getStartR1(const string & word)
{
    // special cases
    if(word.substr(0, 5) == "gener")
        return 5;
    if(word.substr(0, 6) == "commun")
        return 6;
    if(word.substr(0, 5) == "arsen")
        return 5;

    // general case
    int startR1 = firstNonVowelAfterVowel(word, 1);

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
    if(DEBUG) cout << "    startR1: " << startR1 << endl;

    return startR1;
}

int Porter2Stemmer::internal::getStartR2(const string & word, int startR1)
{
    if(startR1 == word.length())
        return startR1;

    int startR2 = firstNonVowelAfterVowel(word, startR1);

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
    if(DEBUG) cout << "    startR2: " << startR2 << endl;

    return startR2;
}

int Porter2Stemmer::internal::firstNonVowelAfterVowel(const string & word, int start)
{
    for(size_t i = start; i < word.length(); ++i)
    {
        if(isVowelY(word[i]) && !isVowelY(word[i - 1]))
            return i + 1;
    }

    return word.length();
}

void Porter2Stemmer::internal::changeY(string & word)
{
    if(word.find_first_of("y") == string::npos)
        return;

    if(word[0] == 'y')
        word[0] = 'Y';

    for(size_t i = 1; i < word.length(); ++i)
    {
        if(word[i] == 'y' && isVowel(word[i - 1]))
            word[i++] = 'Y'; // skip next iteration
    }

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::removeApostrophe(string & word)
{
    if(endsWith(word, "'s'"))
        word = word.substr(0, word.length() - 3);
    else if(endsWith(word, "s'"))
        word = word.substr(0, word.length() - 2);
    else if(endsWith(word, "'"))
        word = word.substr(0, word.length() - 1);

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

/**
  Step 1a:

  sses
    replace by ss 

  ied   ies
    replace by i if preceded by more than one letter, otherwise by ie
    (so ties -> tie, cries -> cri) 

  us   ss
    do nothing

  s
    delete if the preceding word part contains a vowel not immediately before the
    s (so gas and this retain the s, gaps and kiwis lose it) 
*/
bool Porter2Stemmer::internal::step1A(string & word)
{
    const vector<Replacement> first = { Replacement("sses", "ss") };
    if(!replace(first, word, 0))
    {
        if(endsWith(word, "ied") || endsWith(word, "ies"))
        {
            
        }
        else if(endsWith(word, "s") && !endsWith(word, "us") && !endsWith(word, "ss"))
        {

        }
    }

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;

    // special case after step 1a
    return word == "inning" || word == "outing" || word == "canning" || word == "herring" ||
        word == "earring" || word == "proceed" || word == "exceed" || word == "succeed";
}

/**
  Step 1b:

  eed   eedly
      replace by ee if in R1 

  ed   edly   ing   ingly
      delete if the preceding word part contains a vowel, and after the deletion:
      if the word ends at, bl or iz add e (so luxuriat -> luxuriate), or
      if the word ends with a double remove the last letter (so hopp -> hop), or
      if the word is short, add e (so hop -> hope)
*/
void Porter2Stemmer::internal::step1B(string & word, int startR1)
{
    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

/**
  Step 1c:

  Replace suffix y or Y by i if preceded by a non-vowel which is not the first
  letter of the word (so cry -> cri, by -> by, say -> say)
*/
void Porter2Stemmer::internal::step1C(string & word)
{
    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

/**
  Step 2:

  If found and in R1, perform the action indicated. 

  tional:   replace by tion
  enci:   replace by ence
  anci:   replace by ance
  abli:   replace by able
  entli:   replace by ent
  izer   ization:   replace by ize
  ational   ation   ator:   replace by ate
  alism   aliti   alli:   replace by al
  fulness:   replace by ful
  ousli   ousness:   replace by ous
  iveness   iviti:   replace by ive
  biliti   bli:   replace by ble
  ogi:   replace by og if preceded by l
  fulli:   replace by ful
  lessli:   replace by less
  li:   delete if preceded by a valid li-ending
*/
void Porter2Stemmer::internal::step2(string & word, int startR1)
{
    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step3(string & word, int startR1, int startR2)
{
    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step4(string & word, int startR2)
{
    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step5(string & word, int startR1, int startR2)
{
    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

/**
 * Define a short syllable in a word as either (a) a vowel followed by a
 * non-vowel other than w, x or Y and preceded by a non-vowel, or * (b) a vowel
 * at the beginning of the word followed by a non-vowel.
 *
 * A word is called short if it ends in a short syllable, and if R1 is null.
 */
bool Porter2Stemmer::internal::isShort(const string & word, int startR1)
{
    return false;
}

bool Porter2Stemmer::internal::special(string & word)
{
    const std::unordered_map<string, string> exceptions = {
        {"skis", "ski"}, {"skies", "sky"},
        {"dying", "die"}, {"lying", "lie"},
        {"tying", "tie"}, {"idly", "idl"},
        {"gently", "gentl"}, {"ugly", "ugli"},
        {"early", "earli"}, {"only", "onli"},
        {"singly", "singl"} };

    // special cases
    auto ex = exceptions.find(word);
    if(ex != exceptions.end())
    {
        word = ex->second;
        return true;
    }

    // invariants
    return word == "sky" || word == "news" || word == "howe" ||
       word == "atlas" || word == "cosmos" || word == "bias" ||
       word == "andes";
}

bool Porter2Stemmer::internal::replace(const vector<Replacement> & replacements, string & word, int position)
{
    return false;
}

bool Porter2Stemmer::internal::isVowelY(char ch)
{
    return ch == 'e' || ch == 'a' || ch == 'i' ||
        ch == 'o' || ch == 'u' || ch == 'y';
}

bool Porter2Stemmer::internal::isVowel(char ch)
{
    return ch == 'e' || ch == 'a' || ch == 'i' ||
        ch == 'o' || ch == 'u';
}

bool Porter2Stemmer::internal::endsWith(const string & word, const string & str)
{
    return word.length() >= str.length() &&
        word.substr(word.length() - str.length()) == str;
}