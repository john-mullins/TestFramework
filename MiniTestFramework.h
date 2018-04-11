#if !defined(IMG_MiniTestFramework_h_)
#define IMG_MiniTestFramework_h_

#include "assertions.h"
#include "testfailure.h"
#include "TestHelpers.h"
#include <vector>

namespace UnitTests
{
	//#    define PP_CAT(a, b) PP_CAT_OO((a, b))
	//#    define PP_CAT_OO(par) PP_CAT_I ## par
	#    define PP_CAT(a, b) a ## b

    class MiniSuite
    {
    public:
        static MiniSuite& Instance();
        
        class Test
        {
        public:
            Test(const std::string& name, const char * file, int line) : name(name), file(file), line(line) {}
                
            virtual void Run(size_t) const = 0;
            virtual size_t NumTests() const = 0;
            virtual ~Test() {} 
        
            std::string     name;    
            const char *    file;
            int             line;    
        };        
        
        class FunctionTest : public Test
        {
        public:
            FunctionTest(void (*fn)(), const std::string& name, const char * file, int line) 
                :   Test(name, file, line),
                    fn(fn)
            {
            }
            
            virtual void Run(size_t) const override { fn(); }
            virtual size_t NumTests() const override { return 1; }
            
        private:
            void (*fn)();
        };
        
        template<class Container, class Function>
            class ParamFunctionTest : public Test
        {
        public:
            typedef typename Container::value_type						param_type;

            ParamFunctionTest(const Container& cont, const Function& fn, const std::string& name, const char * file, int line)
                :	Test(name, file, line),
                    cont(cont),
                    fn(fn)
            {
            }

            virtual	size_t NumTests() const override 
            {
                return std::distance(cont.begin(), cont.end());
            }

            virtual	void Run(size_t index) const override 
            {
                fn(*(cont.begin() + index));                
            }
            
        private:
            Container cont;
            Function  fn;
        };
	
	#define PRINTF printf
	
        size_t AddTest(Test * test)
        {
            tests.push_back(test);
            return 0;
        }
        
        size_t AddTest(void (*fn)(), const std::string& name, const char * file, int line)
        {
            return AddTest(new FunctionTest(fn, name, file, line));
        }
        
        template<class Container, class Function>
            size_t AddParamTest(const Container& cont, const Function &fn, const char* name, const char * file, int line)
        {
            return AddTest(new ParamFunctionTest<Container, Function>(cont, fn, name, file, line));
        }
        
        template <typename T, size_t N, class Function> 
            size_t AddParamTest( const T (&data)[ N ], const Function &fn, const char * name, const char * file, int line)
        {
            return AddParamTest(std::vector<T>(data, data + N), fn, name, file, line);	
        }        
        
        int RunTests(const std::vector<std::string>& )
        {
            bool verbose = false;
            std::vector<std::pair<std::string, std::string> > failures;
            size_t errors = 0;
            size_t num_tests = 0;
            for (auto&& test : tests)
            {
                for (size_t index = 0; index != test->NumTests(); ++index)
                {
					std::string indexs = test->NumTests() == 1 ? "" : "[" + std::to_string(index) + "]";
                    std::string name = test->name + indexs;
					if (verbose)
					{
					    PRINTF("Running %s ", name.c_str());
					}
                    try
                    {
                        ++num_tests;
                        test->Run(index);
                        PRINTF(!verbose ? "." : "OK\n");
                    }
                    catch (TestFailure& e)
                    {
                        failures.push_back(std::make_pair(name, e.what()));
                        PRINTF(!verbose ? "F" : "FAIL\n");
                    }
                    catch (const std::exception& e)
                    {
                        failures.push_back(std::make_pair(name, e.what()));
                        PRINTF(!verbose ? "E" : "EXCEPTION\n");
                        ++errors;
                    }
                }
                delete test;
                test = nullptr;
            }
            PRINTF("\n");
            if (!failures.empty())
            {
                PRINTF("Failures :-\n");            
                for (auto&& failure : failures)
                {
                    PRINTF("%s  while testing TEST(%s)\n", failure.second.c_str(), failure.first.c_str());
                }
            }
            PRINTF("%d Tests.\n", static_cast<int>(num_tests));
            PRINTF("%d Skipped.\n", 0);
            PRINTF("%d Failures.\n", static_cast<int>(failures.size() - errors));
            PRINTF("%d Errors.\n", static_cast<int>(errors));
            return static_cast<int>(failures.size());
        }
        
    private:
        std::vector<Test*> tests;
    };

	#define TEST(name) void name();																	\
        namespace {																						\
            namespace PP_CAT(unique, __LINE__) {																\
                const size_t ignore_this_warning = UnitTests::MiniSuite::Instance().AddTest(name, #name, __FILE__, __LINE__);  \
            }																							\
        }																								\
        void name()                                                                                     \
    /**/    
    

	#define PARAM_TEST(name, data)																					\
		struct name																									\
		{																											\
			template<typename T>																					\
				void operator()(const T& param) const;																\
		};																											\
		namespace	{																								\
			namespace PP_CAT(unique, __LINE__)	{															\
				const size_t ignore_this_warning = UnitTests::MiniSuite::Instance().AddParamTest(data, name(), #name, __FILE__, __LINE__);				\
			}																										\
		}																											\
		template<typename T> void name::operator()(const T& param) const											\
	/**/        
            

    #define TEST_MAIN()                                                                                               \
        UnitTests::MiniSuite& UnitTests::MiniSuite::Instance()                                                        \
        {                                                                                                             \
            static UnitTests::MiniSuite runner;                                                                       \
            return runner;                                                                                            \
        }                                                                                                             \
        std::string UnitTests::FormatError(const std::string& file, int line, int error)                              \
        {                                                                                                             \
            std::string msg = file;                                                                                   \
            msg += "(" + std::to_string(line) + ")";                                                                  \
            msg += " : error A" + std::to_string(error) + ": ";                                                       \
            return msg;                                                                                               \
        }                                                                                                             \
        int main(int argc, char ** argv)                                                                              \
        {                                                                                                             \
            std::vector<std::string> args;                                                       \
            return UnitTests::MiniSuite::Instance().RunTests(args);                                                   \
        }                                                                                                             \
    /**/

}


#endif

