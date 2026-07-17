#include "tsfi_majormud.h"
#include <stdio.h>
#include <string.h>

int tsfi_mf_zmachine_majormud_init(char *mud_state, int max_len) {
    if (!mud_state || max_len < 128) return -1;
    snprintf(mud_state, max_len, "HP:100|GOLD:50|XP:0|CLASS:Warrior|ROOM:1|MONSTER_HP:30");
    return 0;
}

int tsfi_mf_zmachine_majormud_command(const char *cmd, char *mud_state, char *response_out, int max_len) {
    if (!cmd || !mud_state || !response_out || max_len <= 0) return -1;

    int hp = 100, gold = 50, xp = 0, room = 1, monster_hp = 30;
    char class_name[32] = "Warrior";

    if (sscanf(mud_state, "HP:%d|GOLD:%d|XP:%d|CLASS:%31[^|]|ROOM:%d|MONSTER_HP:%d",
               &hp, &gold, &xp, class_name, &room, &monster_hp) < 6) {
        return -2;
    }

    if (strcmp(cmd, "look") == 0) {
        if (monster_hp > 0) {
            snprintf(response_out, max_len, "You are in Room %d. A wild Goblin (HP: %d) is here!", room, monster_hp);
        } else {
            snprintf(response_out, max_len, "You are in Room %d. It is quiet here.", room);
        }
    } else if (strcmp(cmd, "stats") == 0) {
        snprintf(response_out, max_len, "Character: %s | HP: %d | Gold: %d | XP: %d", class_name, hp, gold, xp);
    } else if (strcmp(cmd, "attack") == 0) {
        if (monster_hp <= 0) {
            snprintf(response_out, max_len, "There is nothing here to attack.");
        } else {
            monster_hp -= 15;
            hp -= 5;
            if (monster_hp <= 0) {
                xp += 20;
                gold += 10;
                snprintf(response_out, max_len, "You attack the Goblin! You slay it and gain 20 XP and 10 Gold!");
            } else {
                snprintf(response_out, max_len, "You attack the Goblin! Goblin HP: %d. Goblin hits you back for 5 damage!", monster_hp);
            }
        }
    } else if (strcmp(cmd, "move") == 0) {
        room++;
        monster_hp = 30;
        snprintf(response_out, max_len, "You move to Room %d.", room);
    } else {
        snprintf(response_out, max_len, "Unknown MajorMUD command: %s", cmd);
    }

    snprintf(mud_state, 128, "HP:%d|GOLD:%d|XP:%d|CLASS:%s|ROOM:%d|MONSTER_HP:%d",
             hp, gold, xp, class_name, room, monster_hp);

    return 0;
}
