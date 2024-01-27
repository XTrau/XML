#include "XML.hpp"

// ####################### Constructors #####################################

bool check_name(const string& s) {
	if (s.length() > 0 && (isalpha(s[0]) || s[0] == ':' || s[0] == '_')) {
		for (int i = 1; i < s.length(); i++)
			if (isalnum(s[i]) == 0 && s[i] != '-' && s[i] != '.' && s[i] != '_' && s[i] != ':')
				return false;
		return true;
	}
	return false;
}

XML::XML(string name) {
	bool good = check_name(name);
	if (!good)
		throw xml_creating_error();
	this->name = name;
	this->text = "";
}

XML::~XML() {
	for (int i = 0; i < children.size(); i++)
		delete children[i];
}

XML::XML(const XML& xml) {
	name = xml.name;
	attributes = xml.attributes;
	text = xml.text;

	for (int i = 0; i < xml.children.size(); i++)
		children.push_back(new XML(*xml.children[i]));
}

// ####################### Add data #####################################

XML* XML::add_element(const string& name) {
	XML* xml = new XML(name);
	children.push_back(xml);
	return xml;
}

void XML::add_attribute(const string& attribute, const string& value) {
	bool good = check_name(attribute);
	if (!good)
		throw xml_attribute_creating_error();

	for (int i = 0; i < attributes.size(); i++)
		if (attributes[i].first == attribute) {
			attributes[i].second = value;
			return;
		}

	attributes.push_back(make_pair(attribute, value));
}

// ####################### Delete data #####################################

void XML::remove_element(int i) {
	if (i < 0 || i >= children.size()) throw xml_bad_index();
	delete children[i];
	children.erase(children.cbegin() + i);
}

void XML::remove_attribute(int i) {
	if (i < 0 || i >= attributes.size()) throw xml_bad_index();
	attributes.erase(attributes.cbegin() + i);
}

// ####################### Get data #####################################

XML* XML::find_element(const string& name, const string& attr, const string& value) {
	for (int i = 0; i < children.size(); i++)
		if (children[i]->name == name)
			for (int j = 0; j < children[i]->attributes.size(); j++)
				if (children[i]->attributes[j].first == attr && children[i]->attributes[j].second == value)
					return children[i];

	return nullptr;
}

XML* XML::operator[](int i) {
	if (i >= 0 && i < children.size())
		return children[i];
	throw xml_bad_index();
}

const XML* XML::operator[](int i) const {
	if (i >= 0 && i < children.size())
		return children[i];
	throw xml_bad_index();
}

int XML::get_attribute_index(const string& attribute_name) const {
	for (int i = 0; i < attributes.size(); i++)
		if (attributes[i].first == attribute_name)
			return i;
	return -1;
}

string XML::get_attribute_name(int i) const {
	if (i < 0 || i >= attributes.size())
		throw xml_bad_index();
	return attributes[i].first;
}

string XML::get_attribute_value(int i) const {
	if (i < 0 || i >= attributes.size())
		throw xml_bad_attribute();
	return attributes[i].second;
}

string XML::get_attribute_value(const string& name) const {
	for (int i = 0; i < attributes.size(); i++)
		if (attributes[i].first == name)
			return attributes[i].second;
	throw xml_bad_attribute();
}

// ############################ Save ##################################

string convert_text(const string& text) {
	const char keys[] = { '&', '<', '"', '>', '\'' };
	const string values[] = { "&amp;", "&lt;", "&quot;", "&gt;", "&#39;" };
	string result = "";

	for (int i = 0; i < text.length(); i++) {
		bool finded = false;
		for (int j = 0; j < 5; j++)
			if (text[i] == keys[j]) {
				result += values[j];
				finded = true;
				break;
			}
		if (!finded)
			result += text[i];
	}

	return result;
}

void save_element_start(XML* xml, ofstream& outFile, bool single, int tabs = 0) {
	outFile << string(tabs, '\t');
	outFile << '<' << xml->name;
	for (int i = 0; i < xml->attributes.size(); i++) {
		string converted_value = convert_text(xml->attributes[i].second);
		outFile << ' ' << xml->attributes[i].first << '=' << '"' << converted_value << '"';
	}
	if (single) outFile << "/";
	outFile << '>';
}

void save_element_end(const string& xml_name, ofstream& outFile, int tabs = 0) {
	outFile << string(tabs, '\t');
	outFile << "</" << xml_name << ">";
}

void save_tree(XML* xml, ofstream& outFile, int cnt = 0) {
	bool single = xml->children.size() == 0 && xml->text.length() == 0;
	save_element_start(xml, outFile, single, cnt);
	if (single) return;

	if (xml->children.size() == 0) {
		outFile << convert_text(xml->text);
		save_element_end(xml->name, outFile);
		return;
	}

	outFile << '\n';
	if (xml->text.length() != 0) {
		outFile << string(' ', cnt) << convert_text(xml->text) << '\n';
	}

	for (int i = 0; i < xml->children.size(); i++) {
		save_tree(xml->children[i], outFile, cnt + 1);
		outFile << '\n';
	}

	save_element_end(xml->name, outFile, cnt);
}

void XML::save(const string& file_name) {
	ofstream outFile(file_name);
	outFile << "<?xml version=\"1.0\"?>\n";
	save_tree(this, outFile);
}

string trim(string& s) {
	int i = 0;
	while (i < s.size() && s[i] == ' ' || s[i] == '\t') i++;
	if (i == s.size()) return "";
	string ans = "";
	char prev = 0;
	while (i < s.size()) {
		if (s[i] == ' ' && prev == ' ') {
			i++;
			continue;
		}
		ans += s[i];
		prev = s[i];
		i++;
	}
	return ans;
}

// ############################ Reading from file ##################################

string parse_text(const string& text) {
	const char keys[] = { '&', '<', '"', '>', '\'' };
	const string values[] = { "&amp;", "&lt;", "&quot;", "&gt;", "&#39;" };
	string result = "";
	for (int i = 0; i < text.size(); i++) {
		if (text[i] == '&') {
			string s = "&";
			while (text[i] != ';') {
				i++;
				s += text[i];
			}
			for (int i = 0; i < 5; i++) {
				if (values[i] == s) {
					result += keys[i];
				}
			}
			continue;
		}
		result += text[i];
	}
	return result;
}

XML XML::parse(const string& file_name) {
	ifstream inputFile(file_name);
	string s;
	getline(inputFile, s);
	if (s != "<?xml version=\"1.0\"?>") throw xml_creating_error();
	stack<XML*> st;
	while (getline(inputFile, s))
	{
		s = trim(s);
		if (s.size() == 0) continue;
		string name = "";
		bool end = false;
		int i = 1;
		if (s[0] != '<') {
			string text = "";
			for (int i = 0; i < s.size(); i++)
				text += s[i];
			st.top()->set_text(text);
			continue;
		}
		else {
			if (s[i] == '/') {
				end = true;
				i++;
			}
			while (i < s.size() && s[i] != ' ' && s[i] != '/' && s[i] != '>') {
				name += s[i];
				i++;
			}
			if (end) {
				if (st.top()->name != name)
					throw xml_parse_error();
				if (st.size() == 1) break;
				st.pop();
				continue;
			}
			XML* xml = new XML(name);
			if (!st.empty())
				xml = st.top()->add_element(name);
			st.push(xml);
			if (s[i] == ' ') {
				while (i < s.size() && s[i] != '/' && s[i] != '>') {
					i++;
					string attr = "";
					string val = "";
					bool flag = false;
					int cnt = 0;
					while (i < s.size() && s[i] != '/' && s[i] != '>') {
						if (s[i] == '=') {
							flag = true;
							i++;
							continue;
						}
						if (s[i] == '"') {
							cnt++;
							i++;
							if (cnt == 2)
								break;
							continue;
						}
						if (flag == false) attr += s[i];
						else val += s[i];
						i++;
					}
					try {
						xml->add_attribute(attr, parse_text(val));
					}
					catch(xml_attribute_creating_error e) {
						throw xml_parse_error();
					}
				}
			}
			if (s[i] == '/') {
				if (st.top()->name != name)
					throw xml_parse_error();
				if (st.size() == 1) break;
				st.pop();
				continue;
			}
			else if (s[i] == '>') {
				if (i + 1 < s.size()) {
					string text = "";
					i++;
					while (i < s.size() && s[i] != '<') {
						text += s[i];
						i++;
					}
					xml->set_text(parse_text(text));
				}
				else
					continue;
			}
			if (s[i] == '<' && s[i + 1] == '/') {
				if (st.top()->name != name)
					throw xml_parse_error();
				if (st.size() == 1) break;
				st.pop();
				continue;
			}
		}
	}
	return *st.top();
}