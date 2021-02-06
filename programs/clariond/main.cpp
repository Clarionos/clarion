#include <appbase/application.hpp>
#include "websock/shared_state.hpp"
#include "websock/listener.hpp"
#include <iostream>

namespace bpo = boost::program_options;
using bpo::options_description;
using bpo::variables_map;
using std::string;
using std::vector;

namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

class http_plugin : public appbase::plugin<http_plugin>
{
   public:
     http_plugin(){};
     ~http_plugin(){};

     APPBASE_PLUGIN_REQUIRES( /*(chain_plugin)*/ );

     virtual void set_program_options( options_description& cli, options_description& cfg ) override
     {
        cfg.add_options()
              ("listen-endpoint", bpo::value<string>()->default_value( "127.0.0.1:9876" ), "The local IP address and port to listen for incoming connections.")
              ("document-root", bpo::value<string>()->default_value( "." ), "Root directory of web server")
     //         ("remote-endpoint", bpo::value< vector<string> >()->composing(), "The IP address and port of a remote peer to sync with.")
     //         ("public-endpoint", bpo::value<string>()->default_value( "0.0.0.0:9876" ), "The public IP address and port that should be advertized to peers.")
              ;
     }

     void plugin_initialize( const variables_map& options ) { 
         std::cout << "initialize net plugin\n"; 
         auto ep = options.at("listen-endpoint").as<string>();
         auto ip = ep.substr( 0, ep.find( ':' ));

         _ip = net::ip::make_address(ip);
         _port = std::stoi(ep.substr( ip.size() + 1, ep.size()));
         _docroot = options.at("document-root").as<string>();
     }

     void plugin_startup()  { 
         std::cout << "starting net plugin \n"; 

         auto ss = std::make_shared<shared_state>(_docroot);
         
         std::make_shared<listener>(
             appbase::app().get_io_service(),
             tcp::endpoint{_ip, _port},
             ss )->run();
             
     }
     void plugin_shutdown() { std::cout << "shutdown net plugin \n"; }

     private:
        net::ip::address _ip;
        uint16_t         _port = 8090;
        string           _docroot;

};


int main( int argc, char** argv ) {
   try {
      auto& a = appbase::app();
      appbase::app().register_plugin<http_plugin>();
      if( !a.initialize( argc, argv ) )
         return -1;
      a.startup();
      a.exec();
   } catch ( const boost::exception& e ) {
      std::cerr << boost::diagnostic_information(e) << "\n";
   } catch ( const std::exception& e ) {
      std::cerr << e.what() << "\n";
   } catch ( ... ) {
      std::cerr << "unknown exception\n";
   }
   std::cout << "exited cleanly\n";
   return 0;
}
