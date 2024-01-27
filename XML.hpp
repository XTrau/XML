#ifndef XML_H
#define XML_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>

using namespace std;

class XML {
public:
	XML(string _name);
	XML(const XML& xml);
	~XML();
	string get_name() const { return name; }
	string get_text() const { return text; }
	void set_text(const string& s) { text = s; }

	int get_children_count() const { return children.size(); }
	XML* operator[](int i);
	const XML* operator[](int i) const;

	XML* add_element(const string& name);
	XML* find_element(const string& name, const string& attr, const string& value);
	void remove_element(int i);

	int get_attributes_count() const { return attributes.size(); }
	void add_attribute(const string& attribute, const string& value);
	string get_attribute_name(int i) const;
	int get_attribute_index(const string& name) const;

	string get_attribute_value(int i) const;
	string get_attribute_value(const string& name) const;
	void remove_attribute(int i);

	void save(const string& file_name);
	static XML parse(const string& file_name);
private:
	string name;
	string text;
	vector<XML*> children;
	vector<pair<string, string>> attributes;
	friend void save_tree(XML* xml, ofstream& outFile, int cnt);
	friend void save_element_start(XML* xml, ofstream& outFile, bool single, int tabs);
};

struct xml_error {
	virtual ~xml_error() {}
	virtual const char* what() const = 0;
};

struct xml_bad_index : xml_error {
	const char* what() const { return "Out of range! (Bad index)"; }
};

struct xml_bad_attribute : xml_error {
	const char* what() const { return "Attribute is not exist"; }
};

struct xml_attribute_creating_error : xml_error {
	const char* what() const { return "Attribute creating error"; }
};

struct xml_creating_error : xml_error {
	const char* what() const { return "XML creating error"; }
};

struct xml_parse_error : xml_error {
	const char* what() const { return "XML parse error!"; }
};


#endif