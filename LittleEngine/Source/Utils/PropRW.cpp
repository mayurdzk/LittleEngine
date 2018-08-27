#include "stdafx.h"
#include <vector>
#include "PropRW.h"
#include "FileRW.h"

std::vector<std::string> Tokenize(const std::string& s, char c) {
	auto end = s.cend();
	auto start = end;

	std::vector<std::string> v;
	for (auto it = s.cbegin(); it != end; ++it) {
		if (*it != c) {
			if (start == end)
				start = it;
			continue;
		}
		if (start != end) {
			v.emplace_back(start, it);
			start = end;
		}
	}
	if (start != end)
		v.emplace_back(start, end);
	return v;
}

std::string PropertiesToString(const std::vector<Property>& vec) {
	std::string fileContents;
	for (const auto& prop : vec) {
		fileContents += (prop.key + ":" + prop.stringValue + "\n");
	}
	return fileContents;
}


bool PropRW::Load(const std::string& filePath) {
	FileRW fileRW(filePath);
	if (!fileRW.Exists()) {
		return false;
	}

	std::vector<std::string> lines = fileRW.Read();
	for (const auto& line : lines) {
		if (line.length() <= 0) continue;
		if (line.c_str()[0] == '#') continue;

		std::vector<std::string> tokens = Tokenize(line, ':');
		if (tokens.size() > 1) {
			SetProp(Property(tokens[0], tokens[1]));
		}
	}
	return true;
}

bool PropRW::Save(const std::string& filePath) const {
	FileRW file(filePath);
	return file.Write(PropertiesToString(properties));
}

Property PropRW::GetProp(const std::string& key) const {
	for (auto& prop : properties) {
		if (prop.key == key) {
			return prop;
		}
	}
	return Property();
}

void PropRW::SetProp(const Property& property) {
	for (auto& prop : properties) {
		if (prop.key == property.key) {
			prop.stringValue = property.stringValue;
			return;
		}
	}
	properties.push_back(property);
}
