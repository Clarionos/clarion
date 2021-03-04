Design:

1. Each node can have many user
2. Each user can publish N messages (from themselves or other users)
3. Each node stores X bytes and prunes oldest messages first
4. Each node is therefore a database of

        struct message {
             from,
             time,
             data
             attention_keys[]  (eg @username )
             attention_messages[]  (eg, reply to...)
        }
        
        struct user {
            key,
            account_name, // assigned by the owner of the node/inviter
            bytes broadcast
        }

        struct identity {
            struct meta_data {
                other_user
                string notes
                vector<address> 
                date   last_update
                signature
            }
            key owner
            display_name, // defined by the identity
            vector<meta_data> metadata
        }

5. Each node has a set of active connections of
     
         struct connect {
            peer_key,
            subscribed_to_keys[]
            subscribed_to_messages[]
         }


Who can connect to your node?

1. only your friends?
    what about someone who wants to send you an email?
    you can authenticate to the server with a proof of work?
      - certain messages are allowed prior to authentication


1. IPFS principles... your host anything you download...
2. You allow friends to upload videos to you



A Peer = A node in the network with a peer key
A User = A user is identified by a key and may exist on multiple peers at the same time

A Peer can be connected to by any other Peer blessed by a User
A Peer has its own key because it operates 'autonomously' from the user
A User keeps their key in the UI layer, never sending their private key to a peer

A browser runs a peer in web assembly that connects to a peer in the cloud
The browser will generate a "peer key" and the user will "bless" the browser peer before it connects



On a new connection, the peer must authenticate itself and the peer it is connecting to:

user, browser, server are all names for public keys that make this documentation easier to follow

```
$user@$browser ----  connects via websock --->  /ipv4/ws/port/$server

browser to server
     login_request {
         onetimekey : $client_connection_session_pubkey
         clientkey  : $browser
         clientcert : {
                expiration : tomorrow,
                clientkey:  $browser
                signature:  $user.sign(expiration+clientkey)
         }
     }

server to browser
     login_challenge {
        onetimekey: $server_connection_session_pubkey
        if( login_request.clientcert is not expired and the user is a valid user of this server )
            auth: $server.sign( dh.get_shared_secret( $server_connection_session_privkey, $browser_connection_session_pubkey ) )
     }

browser to server
     login_response {
        auth: $browser.sign( dh.get_shared_secret( $browser_connection_session_privkey, $server_connection_session_pubkey ) )
     }
```

## What does it mean to be a user of a server?

1. You can connect to the server
2. You can publish messages to the server from your account
3. You can subscribe to messages from identities known by the server
4. You can invite friends to the server
5. You can add 'identities' to the server

## What is an identity?

1. An identity is a "foreign account", aka a public key of a user on another peer
2. You can subscribe/follow an identity of someone who is not a friend
3. You learn about identities from your friends (users) 



## How do new users get created?

All peers have an admin user which uses the peer id... this admin can login and issue an "add user" message

Each user has the permission to create N other users... where N is assigned at the time a user is created and subtracted from the number of users the parent account could create...
This makes "accounts" a "currency" on a particular server.

A user can "replace their account" with a new key, in this way an invite is just giving someone a private key to a temporary account which then "upgrades" to a real account

```
browser to server 
add_user {
   pubkey
   signed server
}
```



## How do users get removed?

A user can be removed by anyone in the chain of invite up to the server.

## What are friends?

Anyone who has an "account" on your server is a friend... 

