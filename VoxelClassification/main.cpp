#include "MainWindow.h"
#include <QtWidgets/QApplication>

// #include <fstream>
// #include <VMUtils/json_binding.hpp>
//
//
// struct FamiliyJSONStruct:public vm::json::Serializable<FamiliyJSONStruct>
// {
// 	VM_JSON_FIELD(std::vector<int>, baba);
// };
//
// struct InputFileJSONStruct:public vm::json::Serializable<InputFileJSONStruct>
// {
// 	VM_JSON_FIELD(double, tall);
// 	VM_JSON_FIELD(std::string, name);
// 	VM_JSON_FIELD(int, age);
// 	VM_JSON_FIELD(FamiliyJSONStruct, familiy);
// }JSON;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow w;
	w.show();
	return a.exec();
}
