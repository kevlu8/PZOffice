#include "include.hpp"

__gnu_pbds::gp_hash_table<ull, std::stack<std::string>> m;
__gnu_pbds::gp_hash_table<std::string, std::string> old_new;
std::unordered_set<std::string> s;

ull hash(const std::string &path) {
	std::ifstream ifs(path);
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	std::string str = buffer.str();
	const std::hash<std::string> hasher;
	return hasher(str);
}

int main(int argc, char *argv[]) {
	for (const auto & entry : std::filesystem::directory_iterator("./ppt/media")) {
		m[hash(entry.path().string())].push(entry.path().filename().string());
	}

	for (const auto & p : m) {
		// insert ones to keep
		s.insert(p.second.top());
	}

	// delete all other files
	for (const auto & entry : std::filesystem::directory_iterator("./ppt/media")) {
		if (s.find(entry.path().filename().string()) == s.end()) {
			std::filesystem::remove(entry.path());
		}
	}

	// rename all files to 1, 2, 3, ...
	int i = 1;
	std::filesystem::create_directory("./ppt/media2");
	for (const auto & entry : std::filesystem::directory_iterator("./ppt/media")) {
		old_new[entry.path().filename().string()] = "image" + std::to_string(i) + entry.path().extension().string();
		std::filesystem::copy(entry.path(), "./ppt/media2/image" + std::to_string(i) + entry.path().extension().string());
		i++;
	}

	std::filesystem::remove_all("./ppt/media");
	std::filesystem::rename("./ppt/media2", "./ppt/media");

	// now we have to replace
	for (const auto & entry : std::filesystem::recursive_directory_iterator("./ppt/slides/_rels")) {
		if (entry.path().extension() == ".rels") {
			std::ifstream ifs(entry.path());
			std::stringstream buffer;
			buffer << ifs.rdbuf();

			std::string str = buffer.str();

			for (auto p : m) { // yes, this does need to create a copy
				std::string orig = p.second.top();
				std::replace(old_new[orig].begin(), old_new[orig].end(), '.', '-');
				while (!p.second.empty()) {
					std::string s = std::regex_replace(p.second.top(), (std::regex)"\\.", "\\.");
					str = std::regex_replace(str, std::regex(s), old_new[orig]);
					p.second.pop();
				}
			}

			str = std::regex_replace(str, std::regex("-"), ".");

			std::ofstream ofs(entry.path());
			ofs << str;

			ifs.close();
			ofs.close();
		}
	}

	return 0;
}