#include <stdio.h>
#include <stdlib.h>

#include "debug.h"


void CheckIt(bool expr,
             int line,
             const char *func_name,
             const char *file_name)
{
    if (!expr)
    {
        ColorPrintf(kRed,
               "\n@@@@@@@@@@@@@@@@@@@@@@@@&&&&&&&&@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@@@@&BGP55555555555555PG#@@@@@@@@@@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@&#BPPGGBBBBBBBBBBBBG5YJJ5B&@@@@@@@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@#B########&&&&&&&&&###BGPPPPGBBB#&&@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@&&@@@&&&&&@&&&&@@@&&&BGB#&@@@@&####BG&@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@@&&@@@@&@@@@@@&&#BBG@@&@@@@@@@@@@GB@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@@&@@@@@@@@@@@&#B#@&BBPPPGB&@@@@&&B&@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@&&@@@@@@@@@@&&G#@B5JJ???YGB@@@@#&@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@&#@@@@@@@@@@&#&##G5YJ???JY5B&@@##@@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@B#&&@@@@@@@&B&B5JJYYJJJJJJ7J5&&P#@@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@&BB&@@@@@@@&#G##GP5YYGGP5PPGG5P@BB@@@@@@@@@@@@@"
               "\n@@@@@@@@@@@@&GG&@@@@@@@#&@@@@@@@&#B#&@@@@@&#@##&@@@@@@@@@@@@"
               "\n@@@@@@@@@@@BBB##&@@@&&#B#GBGGBBGG5JJG#BBBB5#@##@@@@@@@@@@@@@"
               "\n@@@@@@@@@#BBBBB#@@@@##&#PYY5P5PBPB##GPJJ??JB@&B@@@@@@@@@@@@@"
               "\n@@@@@@@@#B&BGB&@@@@&&&@&&&&#B##PB&G&BGB&###@@&##@@@@@@@@@@@@"
               "\n@@@@@@@@#&&B##&@@@&B&@@BB&#P555PP5PP5G&B&@@@#PYB@@@@@@@@@@@@"
               "\n@@@@@@@@&@&&#&@@@&&#B@&JJ##BGGGPGGPPPBBY&@@YJYB@@@@@@@@@@@@@"
               "\n@@@@@&@@@@@&&@@@&#@@&&@PY5PGBBGGGGGGBPJ5@BJJB&@@@@@@@@@@@@@@"
               "\n&&####&@@@@&@@@@&&@@@@@@@BP555PGGGPP5Y#@G75P@@&&@@@@@@@@@@@@"
               "\n#&#&&&&&@@@@@@@@&&@@@@@@@@B?YYJJ55JJ5&#5JP&@&@&&@@@@@@@@@@@@"
               "\n&&&#@&@&&@@@@@@&&#&@@@@@@@#J?GGPGBGB&J7JP&&YYP@@@@@@@@@@@@@@"
               "\n&&&&@@@@@@@@@@@&&&#&@@@@@@#JJY&@@&#GYYPPYY?Y#B5P&@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@@&&&&@@@@@@&GGGYYYJYY555P5JY5YYB#&@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@&5Y555YPG55PPBGY?PG5P@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@G5PP5GGGP5PPPP5PYJB&@@@@@@@@@@@"
               "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#BBG#&#GGBGGGGPGB&@@@@@@@@@@@@@");

        printf("\n#/ Checker spotted a problem in file: %s, function: %s, line: %d\n", file_name, func_name, line);
        abort();
    }
}
