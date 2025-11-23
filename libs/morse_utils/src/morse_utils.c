#include <morse_utils.h>
#include <string.h>

//Pseudo dictionary for converting the characters
// Entry[0] is morse and Entry[1] is the corresponding letter
const char* alphabet[36][8] = {
    {".-", "a"},
    {"-...", "b"}, {"-.-.", "c"}, {"-..", "d"}, {".", "e"}, {"..-.", "f"},
    {"--.", "g" }, {"....", "h"}, {"..", "i"}, {".---", "j"}, {"-.-", "k"},
    {".-..", "l"}, {"--", "m"}, {"-.", "n"}, {"---", "o"}, {".--.", "p"},
    {"--.-", "q"}, {".-.", "r"}, {"...", "s"}, {"-", "t"}, {"..-", "u"}, 
    {"...-", "v"}, {".--", "w"}, {"-..-", "x"}, {"-.--", "y"}, {"--..", "z"},
    {".----", "1"}, {"..---", "2"}, {"...--", "3"}, {"....-", "4"}, {".....", "5"},
    {"-....", "6"}, {"--...", "7"}, {"---..", "8"}, {"----.", "9"}, {"-----", "0"}
};

/**
 * Searches for a match between given char array and letter
 * Returns matching letter as string, returns "?" if none matchedd
 */
char* morse_to_alpha(char morse[5]) {

    for (int symbol = 0; symbol < sizeof(alphabet); symbol++) { //Simply iterate through the alphabet
        if (strcmp(alphabet[symbol][0], morse) == 0) {
            return alphabet[symbol][1];
        }   
    }
    return "?";
}

/**S
 * earches for a match between given letter and a morse code
 * Writes the matching morse code to given char array
*/
void alpha_to_morse(char *alpha, char morse[5]) {
    for (int symbol = 0; symbol < sizeof(alphabet); symbol++) {
        if (strcmp(alphabet[symbol][1], alpha) == 0) {
            strcpy(morse, alphabet[symbol][0]);
            break;
        }
    }
}