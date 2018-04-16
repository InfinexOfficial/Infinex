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

static const int PROTOCOL_VERSION = 10001;
static const int INFINEX_MN_MIN_VERSION = 10000;
static const int INFINIDEX_MIN_VERSION = 10001;

//! initial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 10001;

//! disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION = 10000;

#endif // BITCOIN_VERSION_H
