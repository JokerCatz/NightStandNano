#pragma once
#include <string>
#include "fprovide.h"
#include "opl.h"
#include "player.h"
CPlayer *adplug_players_factory(Copl *opl, const char *ext, const std::string &fn, const CFileProvider &fp);
