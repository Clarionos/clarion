#include <appbase/application.hpp>
#include <iostream>

int main(int argc, char** argv)
{
   try
   {
      auto& a = appbase::app();
      if (!a.initialize(argc, argv))
         return -1;
      a.startup();
      a.exec();
   }
   catch (const boost::exception& e)
   {
      std::cerr << boost::diagnostic_information(e) << "\n";
   }
   catch (const std::exception& e)
   {
      std::cerr << e.what() << "\n";
   }
   catch (...)
   {
      std::cerr << "unknown exception\n";
   }
   std::cout << "exited cleanly\n";
   return 0;
}
