#include "EnemyData.hpp"

EnemyData::EnemyId EnemyData::indx_to_id(int arrayIndx) noexcept
{
	if (arrayIndx == 39)
		return Dante;
	if (arrayIndx >= 20)
		return (EnemyData::EnemyId)(arrayIndx + 3);
	return (EnemyData::EnemyId)arrayIndx;
}

int EnemyData::id_to_indx(int emId) noexcept
{
	if (emId == 55 || emId == 56)
		return 39;
	if (emId == 42)
		return 35;
	if (emId >= 23)
		return emId = 3;
	return emId;
}

EnemyData::EnemyId EnemyData::get_em_id(uintptr_t enemy)
{
	uint32_t classIndex = ((REManagedObject*)enemy)->info->classInfo->type->classIndex;
	switch (classIndex)
	{
		case 0xE123370E: //HellCaina, app.Em0000
			return HellCaina;

		case 0x545B54E0: //Hell Antenora, app.Em0001
			return HellAntenora;

		case 0x8E6F5A81: //Hell Judecca, app.Em0002
			return HellJudecca;

		case 0x67E868E0:
			return Empusa; //Empusa, app.Em01000

		case 0x875A264F:
			return GreenEmpusa; //Green empusa, app.Em0101

		case 0x6A0B5A55:
			return RedEmpusa; //Red empusa, app.Em0102

		case 0x5D74C093:
			return EmpusaQueen; //Empusa Queen, app.Em0103

		case 0x3E97DF8A:
			return Riot; //Riot, app.Em0200

		case 0xB943AAC3:
			return Chaos; //Chaos, app.Em0201

		case 0x934DCB00:
			return Fury; //Fury, app.Em0202

		case 0xFE34C43B:
			return Baphomet; //Baphomet, app.Em0300

		case 0x4B00F7D2:
			return Lusachia; //Lusachia, app.Em0301

		case 0x6FD4BD07:
			return Behemoth; //Behemoth, app.Em0400

		case 0x56D2DF76:
			return Nobody; //Nobody, app.Em0500

		case 0x70DB6DC0:
			return ScudoAngelo; //Scudo Angelo, app.Em0600

		case 0x7C81B539:
			return ProtoAngelo; //Proto Angelo, app.Em0601

		case 0x92896EE3:
			return DeathScrissors; //Death Scrissors, app.Em0700

		case 0xF87AA53F:
			return Hellbat; //Hellbat, app.Em0800

		case 0x139C4FC9:
			return Pyrobat; //Pyrobat, app.Em0801

		case 0x9CC4ECB2:
			return QliphotsTentacle; //Tentacle, app.Em1000

		case 0xCEF41B20:
		{
			auto owner = *(uintptr_t*)(enemy + 0x10);
			if (owner != 0 && wcscmp((const wchar_t*)((uintptr_t)(owner + 0x28)), L"em5000_goliath_M14") == 0)
				return PhantomGoliath;
			return Goliath; //Goliath, app.Em5000, also for phantom
		}

		case 0xDE100B4F:
			return Malphas; //Malphas chicken, app.Em5100_00 (?)

		case 0xB8067EF3:
			return Malphas; //Malphas body, app.Em5100 (?)

		case 0xA210EBE4:
			return Nidhogg; //Nidhogg tentacle, but look like main, app.Em5200_01

		case 0xE9FBCFA9:
		{
			auto owner = *(uintptr_t*)(enemy + 0x10);
			if (owner != 0 && wcscmp((const wchar_t*)((uintptr_t)(owner + 0x28)), L"em5000_goliath_M14") == 0)
				return PhantomArtemis;
			return Artemis; //Artemis, app.Em5300, also for phantom
		}

		case 0xC055E10E:
			return Gilgamesh; //Gilgamesh, app.Em5400

		case 0xE46DA7F8:
			return Gilgamesh;//Gilgamesh, app.Em540010Left

		case 0x3E0582F3:
			return Gilgamesh;//Gilgamesh, app.Em5400_00

		case 0xEFE7FC80:
			return ElderGeryonKnight; //Time horse, app.Em5500

		case 0x111D0DE3:
		{
			auto owner = *(uintptr_t*)(enemy + 0x10);
			if (owner != 0 && wcscmp((const wchar_t*)((uintptr_t)(owner + 0x28)), L"em5501_angelo_gabriello_M14") == 0)
				return PhantomCavaliere;
			return Cavaliere; //Cavaliere, app.Em5501, also for phantom
		}

		case 0x39133012:
			return QliphothRootBoss; //Qliphoth Root tentacle, app.Em5600_00

		case 0x5AA92ED0:
			return QliphothRootBoss; //Qliphoth Root, app.Em5600

		case 0x35FE2BB1:
			return KingCerberus; //Doggo, app.Em5700

		case 0xAFB482BA:
			return Griffon; //Parrot, app.enemy.em5800.Em5800

		case 0x2FE917F4:
			return Shadow; //Cat, app.enemy.em5801.Em5801

		case 0xCBA105A4:
			return Nightmare; //Golem, app.enemy.em5802.Em5802

		case 0x0AA959C6:
			return Urizen1; //Urizen1, app.Em5900

		case 0xB5478268:
			return Urizen2; //Urizen2, app.Em5901

		case 0xE70D56C0:
			return Urizen3; //Urizen3, app.Em5902

		case 0x82E2961D:
			return Vergil; //Vergil Pl0300, em6000, app.player.pl0300.PlayerVergil

			//case 0xCEF41B20:
			//	return 39; //Phantom Goliath, app.Em5000
			//case 0xE9FBCFA9:
			//	return 40; //Phantom Artemis, app.Em5300
			//case 0x111D0DE3:
			//	return 41; //Phantom Cavaliere, app.Em5501

		case 0xE4EAAABA:
			return Dante; //GUNTRICKSWORDROYALGUNTRICKGUNTRICKSWORD, em6130, app.PlayerDante

	default:
		return None;
	}
}
