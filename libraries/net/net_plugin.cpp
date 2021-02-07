#include <clarion/net/net_plugin.hpp>
#include <clarion/net/listener.hpp>
#include <clarion/net/shared_state.hpp>

#include <iostream>

using std::string;
using std::vector;

namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

namespace cl { namespace net {

     static appbase::abstract_plugin& _clarion_net_plugin = appbase::app().register_plugin<net_plugin>();

     void net_plugin::set_program_options( options_description& cli, options_description& cfg )
     {
        cfg.add_options()
              ("http-listen-endpoint", bpo::value<string>()->default_value( "127.0.0.1:6789" ), "The local IP address and port to listen for incoming connections.")
              ("document-root", bpo::value<string>()->default_value( "." ), "Root directory of web server")
     //         ("remote-endpoint", bpo::value< vector<string> >()->composing(), "The IP address and port of a remote peer to sync with.")
     //         ("public-endpoint", bpo::value<string>()->default_value( "0.0.0.0:9876" ), "The public IP address and port that should be advertized to peers.")
              ;
     }


     void net_plugin::plugin_initialize( const variables_map& options ) { 
         std::cout << "initialize net plugin\n"; 
         auto ep = options.at("http-listen-endpoint").as<string>();
         auto ip = ep.substr( 0, ep.find( ':' ));

         _ip = asio::ip::make_address(ip);
         _port = std::stoi(ep.substr( ip.size() + 1, ep.size()));
         _docroot = options.at("document-root").as<string>();
     }

     void net_plugin::plugin_startup()  { 
         std::cout << "starting net plugin \n"; 

         auto ss = std::make_shared<shared_state>(_docroot);
         
         std::make_shared<listener>(
             appbase::app().get_io_service(),
             tcp::endpoint{_ip, _port},
             ss )->run();
             
     }
     void net_plugin::plugin_shutdown()  { 
         std::cout << "shutdown net plugin \n"; 
     }
} }
