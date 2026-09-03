#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <cstdio>
#include <cctype>
#include <algorithm>

struct Swatch {
	std::string hex; //"RRGGBB", no '#'
	float rgb[3];
};

//returns false if not valid hex
static bool HexToRGB(const std::string& hex, float out[3]) {
	if (hex.size() != 6) return false;
	for (char c : hex) if (!std::isxdigit((unsigned char)c)) return false;
	unsigned int v = (unsigned int)std::stoul(hex, nullptr, 16);
	out[0] = ((v >> 16) & 0xFF) / 255.0f;
	out[1] = ((v >> 8) & 0xFF) / 255.0f;
	out[2] = (v & 0xFF) / 255.0f;
	return true; //unlike you dad, it returns
}

static std::string RGBToHex(const float rgb[3]) {
	char buf[8];
	std::snprintf(buf, sizeof(buf), "%02X%02X%02X",
		(int)(std::clamp(rgb[0], 0.f, 1.f) * 255.0f + 0.5f),
		(int)(std::clamp(rgb[1], 0.f, 1.f) * 255.0f + 0.5f),
		(int)(std::clamp(rgb[2], 0.f, 1.f) * 255.0f + 0.5f));
	return buf;
}

int main() {
	if (!glfwInit()) return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(900, 650, "Color Picker", nullptr, nullptr);
	if (!window) { glfwTerminate(); return 1; }
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync, and you can't disable it

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	//state
	int maxColors = 5; //adjustable
	char hexBuf[8] = "357AB7";
	float curRGB[3];
	HexToRGB(hexBuf, curRGB);
	std::vector<Swatch> palette;
	bool showCompare = false;
	std::string errorMsg;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_FirstUseEver);
		ImGui::Begin("Color Picker");

		ImGui::InputInt("Max colors to compare", &maxColors);
		maxColors = std::max(maxColors, 2); //forced at 2, even you can't fuck this up

		ImGui::Separator();

		//input sync hex and rbg
		if (ImGui::InputText("Hex", hexBuf, sizeof(hexBuf), ImGuiInputTextFlags_CharsUppercase)) {
			std::string h = hexBuf;
			if (!h.empty() && h[0] == '#') h = h.substr(1);
			if (HexToRGB(h, curRGB)) errorMsg.clear();
			else errorMsg = "invalid hex";
		}

		//same same but different
		if (ImGui::ColorEdit3("RGB", curRGB)) {
			std::string h = RGBToHex(curRGB);
			std::snprintf(hexBuf, sizeof(hexBuf), "%s", h.c_str());
			errorMsg.clear();
		}

		ImGui::ColorButton("preview", ImVec4(curRGB[0], curRGB[1], curRGB[2], 1.0f), 0, ImVec2(60, 60));

		if (!errorMsg.empty()) {
			ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", errorMsg.c_str());
		}

		bool atMax = (int)palette.size() >= maxColors;
		ImGui::BeginDisabled(atMax || !errorMsg.empty());
		if (ImGui::Button("Add to comparison")) {
			Swatch s;
			s.hex = hexBuf;
			s.rgb[0] = curRGB[0]; s.rgb[1] = curRGB[1]; s.rgb[2] = curRGB[2];
			palette.push_back(s);
		}
		ImGui::EndDisabled();
		if (atMax) { ImGui::SameLine(); ImGui::TextDisabled("(max reached)"); }

		ImGui::Separator();
		ImGui::Text("Palette (%d/%d)", (int)palette.size(), maxColors);

		//dispaly palette
		int removeIdx = -1;
		for (int i = 0; i < (int)palette.size(); i++) {
			ImGui::PushID(i);
			ImGui::ColorButton("##sw", ImVec4(palette[i].rgb[0], palette[i].rgb[1], palette[i].rgb[2], 1.0f), 0, ImVec2(30, 30));
			ImGui::SameLine();
			ImGui::Text("#%s", palette[i].hex.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("remove")) removeIdx = i;
			ImGui::PopID();
		}
		if (removeIdx >= 0) palette.erase(palette.begin() + removeIdx);

		ImGui::Separator();
		ImGui::BeginDisabled((int)palette.size() < 2);
		if (ImGui::Button(showCompare ? "Hide comparison" : "Compare side by side")) showCompare = !showCompare;
		ImGui::EndDisabled();

		ImGui::End();

		if (showCompare) {
			//ImGui::SetNextWindowSize(ImVec2(150.0f * palette.size() + 40, 220), ImGuiCond_Always); //why forcing the size ?
			ImGui::Begin("Comparison", &showCompare);
			float avail = ImGui::GetContentRegionAvail().x;
			float swatchSize = avail / palette.size() - ImGui::GetStyle().ItemSpacing.x;
			swatchSize = std::max(swatchSize, 40.0f); //min size

			for (int i = 0; i < (int)palette.size(); i++) {
				ImGui::BeginGroup();
				ImGui::ColorButton(("##cmp" + std::to_string(i)).c_str(),
					ImVec4(palette[i].rgb[0], palette[i].rgb[1], palette[i].rgb[2], 1.0f),
					0, ImVec2(swatchSize, swatchSize));
				ImGui::Text("#%s", palette[i].hex.c_str());
				ImGui::Text("%d,%d,%d",
					(int)(palette[i].rgb[0] * 255), (int)(palette[i].rgb[1] * 255), (int)(palette[i].rgb[2] * 255));
				ImGui::EndGroup();
				if (i + 1 < (int)palette.size()) ImGui::SameLine();
			}
			ImGui::End();
		}

		ImGui::Render();
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		glViewport(0, 0, w, h);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}