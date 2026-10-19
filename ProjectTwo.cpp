
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cctype>

using namespace std;

// Structure to store course information
struct Course {
	string courseNumber;
	string courseTitle;
	vector<string> prerequisites;
};

// Structure for a binary search tree node
struct Node {
	Course course;
	Node* left;
	Node* right;
};

// Remove extra whitespace from the beginning and end of a string
string trim(const string& text) {
	size_t start = text.find_first_not_of(" \t\r\n");

	if (start == string::npos) {
		return "";
	}

	size_t end = text.find_last_not_of(" \t\r\n");
	return text.substr(start, end - start + 1);
}

// Convert course numbers to uppercase for consistent searching
string toUpper(string text) {
	for (char& character : text) {
		character = static_cast<char>(
			toupper(static_cast<unsigned char>(character))
			);
	}
	return text;
}

// Binary search tree for storing course information
class BinarySearchTree {
private:
	Node* root;

	// Insert a course into the correct location in the tree
	void addNode(Node* node, const Course& course) {
		if (course.courseNumber < node->course.courseNumber) {
			if (node->left == nullptr) {
				node->left = new Node{ course, nullptr, nullptr };
			}
			else {
				addNode(node->left, course);
			}
		}
		else if (course.courseNumber > node->course.courseNumber) {
			if (node->right == nullptr) {
				node->right = new Node{ course, nullptr, nullptr };
			}
			else {
				addNode(node->right, course);
			}
		}
	}

	// Print courses in alphanumeric order using in-order traversal
	void inOrder(Node* node) const {
		if (node == nullptr) {
			return;
		}

		inOrder(node->left);
		cout << node->course.courseNumber << ", "
			<< node->course.courseTitle << endl;
		inOrder(node->right);
	}

	// Delete all nodes to release allocated memory
	void destroyTree(Node* node) {
		if (node == nullptr) {
			return;
		}

		destroyTree(node->left);
		destroyTree(node->right);
		delete node;
	}

public:
	// Initialize an empty tree
	BinarySearchTree() {
		root = nullptr;
	}

	// Clean up the tree when the object is destroyed
	~BinarySearchTree() {
		clear();
	}

	// Remove all courses from the tree
	void clear() {
		destroyTree(root);
		root = nullptr;
	}

	// Insert a new course
	void insert(const Course& course) {
		if (root == nullptr) {
			root = new Node{ course, nullptr, nullptr };
		}
		else {
			addNode(root, course);
		}
	}

	// Search for a course by its course number
	const Course* search(const string& courseNumber) const {
		Node* current = root;

		while (current != nullptr) {
			if (courseNumber == current->course.courseNumber) {
				return &current->course;
			}
			else if (courseNumber < current->course.courseNumber) {
				current = current->left;
			}
			else {
				current = current->right;
			}
		}

		return nullptr;
	}

	// Print all courses in alphanumeric order
	void printCourseList() const {
		inOrder(root);
	}
};

// Read and validate the CSV file before loading courses into the BST
bool loadCourseData(const string& fileName, BinarySearchTree& tree) {
	ifstream file(fileName);

	if (!file.is_open()) {
		cout << "Error: Unable to open file " << fileName << endl;
		return false;
	}

	vector<vector<string>> courseData;
	unordered_set<string> courseNumbers;
	string line;
	int lineNumber = 0;

	// Read each line and separate the comma-delimited values
	while (getline(file, line)) {
		++lineNumber;

		if (trim(line).empty()) {
			continue;
		}

		stringstream stream(line);
		string parameter;
		vector<string> parameters;

		while (getline(stream, parameter, ',')) {
			parameter = trim(parameter);

			// Ignore empty prerequisite fields
			if (parameters.size() >= 2 && parameter.empty()) {
				continue;
			}

			if (parameters.size() == 1) {
				parameters.push_back(parameter);
			}
			else {
				parameters.push_back(toUpper(parameter));
			}
		}

		// Each course must have a number and title
		if (parameters.size() < 2 ||
			parameters[0].empty() ||
			parameters[1].empty()) {
			cout << "Error: Invalid course data on line "
				<< lineNumber << "." << endl;
			return false;
		}

		// Preserve the original capitalization of the course title
		parameters[1] = trim(line.substr(
			line.find(',') + 1,
			line.find(',', line.find(',') + 1) == string::npos
			? string::npos
			: line.find(',', line.find(',') + 1)
			- line.find(',') - 1
		));

		if (!courseNumbers.insert(parameters[0]).second) {
			cout << "Error: Duplicate course number "
				<< parameters[0] << "." << endl;
			return false;
		}

		courseData.push_back(parameters);
	}

	file.close();

	if (courseData.empty()) {
		cout << "Error: The course data file is empty." << endl;
		return false;
	}

	// Validate every prerequisite against the course numbers in the file
	for (const vector<string>& parameters : courseData) {
		for (size_t i = 2; i < parameters.size(); ++i) {
			if (parameters[i].empty() ||
				courseNumbers.find(parameters[i]) == courseNumbers.end()) {
				cout << "Error: Invalid prerequisite for "
					<< parameters[0] << "." << endl;
				return false;
			}
		}
	}

	// All data is valid, so replace the existing tree
	tree.clear();

	for (const vector<string>& parameters : courseData) {
		Course course;
		course.courseNumber = parameters[0];
		course.courseTitle = parameters[1];

		for (size_t i = 2; i < parameters.size(); ++i) {
			course.prerequisites.push_back(parameters[i]);
		}

		tree.insert(course);
	}

	cout << "Course data loaded successfully." << endl;
	return true;
}

// Display the selected course and its prerequisites
void printCourse(const BinarySearchTree& tree,
	const string& courseNumber) {
	const Course* course = tree.search(courseNumber);

	if (course == nullptr) {
		cout << "Course not found." << endl;
		return;
	}

	cout << course->courseNumber << ", "
		<< course->courseTitle << endl;

	cout << "Prerequisites: ";

	if (course->prerequisites.empty()) {
		cout << "None" << endl;
		return;
	}

	for (size_t i = 0; i < course->prerequisites.size(); ++i) {
		const Course* prerequisite = tree.search(
			course->prerequisites[i]
		);

		if (i > 0) {
			cout << ", ";
		}

		if (prerequisite != nullptr) {
			cout << prerequisite->courseNumber << " - "
				<< prerequisite->courseTitle;
		}
	}

	cout << endl;
}

// Display the advising assistance program menu
void displayMenu() {
	cout << endl;
	cout << "Welcome to the course planner." << endl;
	cout << "1. Load Data Structure" << endl;
	cout << "2. Print Course List" << endl;
	cout << "3. Print Course" << endl;
	cout << "9. Exit" << endl;
	cout << "What would you like to do? ";
}

int main() {
	BinarySearchTree tree;
	bool dataLoaded = false;
	string input;

	while (true) {
		displayMenu();
		getline(cin, input);
		input = trim(input);

		if (input == "1") {
			string fileName;

			cout << "Enter the course data file name: ";
			getline(cin, fileName);

			fileName = trim(fileName);

			if (fileName.empty()) {
				cout << "Error: File name cannot be empty." << endl;
				continue;
			}

			if (loadCourseData(fileName, tree)) {
				dataLoaded = true;
			}
		}
		else if (input == "2") {
			if (!dataLoaded) {
				cout << "Please load the course data first." << endl;
			}
			else {
				cout << endl;
				cout << "Here is a sample schedule:" << endl;
				tree.printCourseList();
			}
		}
		else if (input == "3") {
			if (!dataLoaded) {
				cout << "Please load the course data first." << endl;
			}
			else {
				string courseNumber;

				cout << "What course do you want to know about? ";
				getline(cin, courseNumber);

				courseNumber = toUpper(trim(courseNumber));
				printCourse(tree, courseNumber);
			}
		}
		else if (input == "9") {
			cout << "Thank you for using the course planner!" << endl;
			break;
		}
		else {
			cout << "Invalid option. Please enter 1, 2, 3, or 9."
				<< endl;
		}
	}

	return 0;
}
