#pragma once

namespace cl {

    class node {
        void listen_websocket( ... ) {
            /// create new listener
        }

        void on_message( const id_type& from_peer_id, const message& m ) {
            /// validate the message
            /// is this a new message...
            /// append it to our database...
            /// forward this message to any relevant peers
        }

        void send_message( const id_type& to_peer_id, const message& m ) {
            /// look up peer
            /// lock shared ptr from weak
            /// send message
        }

        struct peer {
            weak_ptr<peer_session>
            id_type peer_id;
        };

        struct peer_users {
            id_type peer_id;
            id_type user_id;
        };

        struct user_follower {
            id_type   user_id;
            id_type   follow_id;
            uint64_t  last_messsage_time;
            uint32_t  last_message_seq;
        };

        struct identity {
            id_type     id;
            uint64_t    last_messsage_time;
            uint32_t    last_message_seq;
        };

        // boost::multi_index<peer_session*,
        //        indexed by id>
        
    };

    struct message {

    };

    class peer_session {
        public:
            virtual void connect( );
            virtual void send( const message& );

        protected:
            void on_read_message( const message& m );
            void on_send_message();
            void on_connect( );
            void on_error();
    };

    class peer_ws_connection : public peer_session {
        public:

    };

} // namespace cl
