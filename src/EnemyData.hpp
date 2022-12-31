#pragma once
#include <memory>
#include <array>
#include "sdk/DMC5.hpp"

namespace EnemyData
{
	enum EnemyId : int32_t
	{
		None = -1,
		HellCaina = 0,
		HellAntenora = 1,
		HellJudecca = 2,
		Empusa = 3,
		GreenEmpusa = 4,
		RedEmpusa = 5,
		EmpusaQueen = 6,
		Riot = 7,
		Chaos = 8,
		Fury = 9,
		Baphomet = 10,
		Lusachia = 11,
		Behemoth = 12,
		Nobody = 13,
		ScudoAngelo = 14,
		ProtoAngelo = 15,
		DeathScrissors = 16,
		Hellbat = 17,
		Pyrobat = 18,
		QliphotsTentacle = 19,
		Goliath = 23,
		Malphas = 24,
		Nidhogg = 25,
		Artemis = 26,
		Gilgamesh = 27,
		ElderGeryonKnight = 28,
		Cavaliere = 29,
		QliphothRootBoss = 30,
		KingCerberus = 31,
		Griffon = 32,
		Shadow = 33,
		Nightmare = 34,
		Urizen1 = 35,
		Urizen2 = 36,
		Urizen3 = 37,
		Vergil = 38,
		PhantomGoliath = 39,
		PhantomArtemis = 40,
		PhantomCavaliere = 41,
		Dante = 55
	};

	inline const std::array<const char*, 40> const *get_em_names() noexcept
	{
	   static std::array<const char*, 40> emNames = {
	  "Hell Caina", //0
	  "Hell Antenora",//1
	  "Hell Judecca", //2
	  "Empusa",//3
	  "Green Empusa",//4
	  "Red Empusa", //5
	  "Empusa Queen",//6
	  "Riot", //7
	  "Chaos", //8
	  "Fury", //9
	  "Baphomet",//10 
	  "Lusachia", //11
	  "Behemoth", //12
	  "Nobody", //13
	  "Scudo Angelo",//14, 
	  "Proto Angelo", //15
	  "Death Scrissors",//16
	  "Hellbat", //17
	  "Pyrobat", //18
	  "Qliphoth's tentacle",//19 
	  "Goliath", //20
	  "Malphas", //21
	  "Nidhogg", //22
	  "Artemis", //23
	  "Gilgamesh", //24
	  "Elder Geryon Knight",//25
	  "Cavaliere", //26
	  "Qliphoth Root Boss",//27 
	  "King Cerberus", //28
	  "Griffon", //29
	  "Shadow", //30
	  "Nightmare", //31
	  "Urizen 1",//32
	  "Urizen 2",//33
	  "Urizen 3",//34
	  "Vergil", //35
	  "Phantom Goliath",//36
	  "Phantom Artemis",//37
	  "Phantom Cavaliere",//38
	  "Dante",//39 - emids: 55 - m19/20, 56 - m21
	  //id 42 - Vergil M20/M21 ingame only
		};
		return &emNames;
	}

	EnemyId indx_to_id(int arrayIndx) noexcept;

	int id_to_indx(int emId) noexcept;

	EnemyId get_em_id(uintptr_t enemy);
}