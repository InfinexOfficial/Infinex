// Copyright (c) 2012-2014 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_VERSION_H
#define BITCOIN_VERSION_H

/**
 * network protocol versioning
 */

static const int PROTOCOL_VERSION = 10000;

//! initial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 10000;

//! disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION = 10000;

//! nTime field added to CAddress, started by default;
static const int CADDR_TIME_VERSION = 10000;

//! BIP 0031, pong message, started by default
static const int BIP0031_VERSION = 10000;

//! "mempool" command, enhanced "getdata" behavior, started by default
static const int MEMPOOL_GD_VERSION = 10000;

//! "filter*" commands are disabled without NODE_BLOOM after and including this version
static const int NO_BLOOM_VERSION = 10000;

//! "sendheaders" command and announcing blocks with headers, started by default
static const int SENDHEADERS_VERSION = 10000;

#endif // BITCOIN_VERSION_H
