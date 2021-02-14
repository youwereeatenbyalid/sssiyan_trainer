
#include "BpStageJump.hpp"
#include "PlayerTracker.hpp"
#include <ctime>

uintptr_t BpStageJump::jmp_ret{NULL};
uintptr_t BpStageJump::jmp_jne{NULL};
bool BpStageJump::cheaton{NULL};
int bpstage = 1;
bool BpStageJump::randombosses = false;
bool BpStageJump::bossrush = false;
bool BpStageJump::endless = false;
int BpStageJump::palacearray[100] = {};
int BpStageJump::counter = 0;
int BpStageJump::bossarray[8] = {};
bool retrystage;
bool randomizer;

int seed = 0;

bool randomStageToggle;
int comboboxindex = 0;
int BpStageJump::palace_type = BpStageJump::RANDOM;

// clang-format off
// only in clang/icl mode on x64, sorry

int BpStageJump::random_generator(int low, int high)
{
	return low + (std::rand() % (high - low + 1));
}

// random_generator(1,8), case switch to return a boss floor
int BpStageJump::return_boss_floor()
{
	switch (BpStageJump::random_generator(1, 8)) {
	case 1:
		return 20;
	case 2:
		return 40;
	case 3:
		return 60;
	case 4:
		return 80;
	case 5:
		return 90;
	case 6:
		return 98;
	case 7:
		return 99;
	case 8:
		return 100;
	default:
		return 10;
	}
}

// return non-boss bp floor
int BpStageJump::return_normal_floor()
{
	int return_floor;
	do
	{
		return_floor = random_generator(1, 97);
	} while (return_floor == 20
		|| return_floor == 40
		|| return_floor == 60
		|| return_floor == 80
		|| return_floor == 90);
	return return_floor;
}

// Reset counter, palacearray,bossarray
void BpStageJump::reset_palace()
{
	for (int i = 0; i < 100; i++) {
		BpStageJump::palacearray[i] = i + 1;
	}
	BpStageJump::bossarray[0] = 20;
	BpStageJump::bossarray[1] = 40;
	BpStageJump::bossarray[2] = 60;
	BpStageJump::bossarray[3] = 80;
	BpStageJump::bossarray[4] = 90;
	BpStageJump::bossarray[5] = 98;
	BpStageJump::bossarray[6] = 99;
	BpStageJump::bossarray[7] = 100;
	BpStageJump::counter = 0;
	// TODO: Add your implementation code here.
}

// randomize values in an array via swapping
void BpStageJump::randomize_array(int* array_param, int range_low, int range_high, int rand_low, int rand_high)
{
	// TODO: Add your implementation code here.
	for (int i = range_low; i < range_high; i++) {
		int temp = array_param[i];
		int rand = random_generator(rand_low, rand_high);
		array_param[i] = array_param[rand];
		array_param[rand] = temp;
	}
}

void BpStageJump::randomize_array(int* array_param, int range_low, int range_high)
{
	for (int i = range_low; i < range_high; i++) {
		int temp = array_param[i];
		int rand = return_normal_floor();
		array_param[i] = array_param[rand - 1];
		array_param[rand] = temp;
	}
}

// Generate a new palace scenario
void BpStageJump::generate_palace(int seed)
{
	reset_palace();
	if (seed != 0){
		std::srand(seed);
	}else{
		std::srand((unsigned int)time(NULL));
	}
	switch (BpStageJump::palace_type) {
	case palace_type_enum::PARTIAL:
		randomize_array(palacearray, 0, 19);
		randomize_array(palacearray, 20, 39);
		randomize_array(palacearray, 40, 59);
		randomize_array(palacearray, 60, 79);
		randomize_array(palacearray, 80, 89);
		randomize_array(palacearray, 90, 97);
		break;
	case palace_type_enum::BALANCED:
		randomize_array(palacearray, 0, 19, 0, 18);
		randomize_array(palacearray, 20, 39, 20, 38);
		randomize_array(palacearray, 40, 59, 40, 58);
		randomize_array(palacearray, 60, 79, 60, 78);
		randomize_array(palacearray, 80, 89, 80, 88);
		randomize_array(palacearray, 90, 97, 90, 96);
		break;
	case palace_type_enum::RANDOM:
	default:
		randomize_array(palacearray, 0, 99, 1, 100);
		break;
	}
	if (BpStageJump::palace_type == palace_type_enum::PARTIAL or BpStageJump::palace_type == palace_type_enum::BALANCED) {
		if (randombosses)
			randomize_array(bossarray, 0, 7, 0, 7);
		palacearray[19] = bossarray[0];
		palacearray[39] = bossarray[1];
		palacearray[59] = bossarray[2];
		palacearray[79] = bossarray[3];
		palacearray[89] = bossarray[4];
		palacearray[97] = bossarray[5];
		palacearray[98] = bossarray[6];
		palacearray[99] = bossarray[7];
	}
}

// get the next floor
int BpStageJump::next_floor()
{
	counter++;
	if (endless) {
		if (palace_type == BpStageJump::RANDOM) {
			return random_generator(1, 100);
		}
		else {
			if (counter % 20 != 0) {
				return return_normal_floor();
			}
			else {
				return return_boss_floor();
			}
		}
	}
	else {
		if (bossrush) {
			if (counter > 8) {
				return 101;
			}
			else {
				return bossarray[counter - 1];
			}
		}
		else {
			if (counter > 100) {
				return 101;
			}
			else {
				return palacearray[counter - 1];
			}
		}
	}
}

static naked void detour() {
	__asm {
        // cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        // jne code
		cmp byte ptr [BpStageJump::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        //cmp byte ptr [bossrush], 1
        //je bossrushstart
        cmp byte ptr [randomStageToggle], 1
        je randomstagestart

        push rax
        mov eax, [bpstage]
        mov [rbx+68h], eax
        pop rax

        cmp byte ptr [retrystage], 1
        je retcode

        mov byte ptr [BpStageJump::cheaton], 00
        jmp retcode

    randomstagestart:
        push rax
        call BpStageJump::next_floor
        mov [rbx+68h], al
        pop rax
        jmp retcode

    code:
        cmp [rbx+79h], cl
        jne jmp_jne
        inc byte ptr [rbx+68h]
    retcode:
        mov rax, [rdi+50h]
        jmp qword ptr [BpStageJump::jmp_ret]

    jmp_jne:
        jmp qword ptr [BpStageJump::jmp_jne]
	}
}

// clang-format on

void BpStageJump::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> BpStageJump::on_initialize() {
  init_check_box_info();

  ischecked               = &BpStageJump::cheaton;
  onpage                  = bloodypalace;
  full_name_string        = "Bp Stage Jump (+)";
  author_string           = "SSSiyan";
  description_string      = "Allows you to skip to a BP stage of your choosing.\n"
                            "Thanks HitchHiker for making the randomizer!";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "38 4B 79 75 03");
  if (!addr) {
    return "Unable to find BpStageJump pattern.";
  }

  generate_palace(0);
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 12)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize BpStageJump";
  }

  BpStageJump::jmp_jne = addr.value() + 8;

  return Mod::on_initialize();
}

void BpStageJump::on_config_load(const utility::Config& cfg) {
  bossrush   = cfg.get<bool>("bp_jump_boss_rush").value_or(false);
  retrystage = cfg.get<bool>("bp_jump_retry_stage").value_or(false);
  randomStageToggle = cfg.get<bool>("bp_jump_random_stage").value_or(false);
}
void BpStageJump::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("bp_jump_boss_rush", bossrush);
  cfg.set<bool>("bp_jump_retry_stage", retrystage);
  cfg.set<bool>("bp_jump_random_stage", randomStageToggle);
}

void BpStageJump::on_draw_ui() {
    ImGui::Spacing();
    ImGui::TextWrapped("Tick to keep retrying whichever stage you are on without continuing");
    ImGui::Checkbox("Retry Current Stage", &retrystage);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Tick before starting BP to skip non boss rooms");
    ImGui::Checkbox("Boss Rush", &bossrush);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextWrapped("Tick to be given random stages");
    ImGui::Checkbox("Random Stage", &randomStageToggle);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

	ImGui::TextWrapped("Play through an infinite bloody palace");
	ImGui::Checkbox("Endless", &endless);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::TextWrapped("Bosses will appear in a random order");
	ImGui::Checkbox("Randomized boss order", &randombosses);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Combo("Palace Type",&palace_type,"Balanced Random Palace\0Partially Random Palace\0Truly Random Palace\0");

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Randomize Palace")){
		generate_palace(seed);
	}

	ImGui::InputInt("Palace Seed",&seed);

    ImGui::TextWrapped("If Boss Rush and Random Stage are unticked, this will teleport you to the room of your choosing when first entering BP or changing stage");
    ImGui::SliderInt("##BP Stage Slider", &bpstage, 1, 101);
}

void BpStageJump::on_draw_debug_ui(){
	
}