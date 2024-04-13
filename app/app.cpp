#include "app.hpp"

bool App::Entry() {
	bool error_present = false;
	
	error_present = Render::Start();
	error_present = App::Cleanup();
	
	return error_present;
}

bool App::Cleanup() {
	return 0;
}