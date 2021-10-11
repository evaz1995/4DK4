#include<fstream>
#include<string>

using namespace std;

namespace utils{
	class FileIO {
	private:
		string FileName;
		ofstream OUT;
	public:
		FileIO(const string& name, const int mode) :
			FileName(name)
		{
			OUT.open(name, mode);
		}

		~FileIO()
		{
			OUT.close();
		}

		void fprint(const char* line)
		{
			OUT << line;
		}

	};
}
