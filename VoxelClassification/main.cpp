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
	// std::ifstream json(R"(C:\Users\hxy\Desktop\json.json)");
	// if(json.is_open() == false)
	// {
	// 	vm::println("failed to open file" );
	// }
	// try
	// {
	// 	json >> JSON;
	// }
	// catch (std::exception & e)
	// {
	// 	vm::println("{}", e.what());
	// }
	//
	// std::stringstream ss;
	// //ss << JSON;
	// vm::json::Writer writer;
	// writer.write(std::cout, JSON);
	//
	// std::ofstream out(R"(C:\Users\hxy\Desktop\json1.json)");
	// writer.write(out, JSON);
	// out.close();
	//vm::println("json : {}", JSON);
	

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
