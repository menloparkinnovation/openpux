/*
 * Copyright (C) 2015 Menlo Park Innovation LLC
 *
 * This is a trade secret, unpublished work of original
 * authorship. This copyright grants no license for any
 * purpose, express or implied.
 *
 *  Date: 01/26/2015
 *  File: DweetChannel.h
 *
 *  Dweet channel base class.
 *
 *  Used for Smartpux DWEET's.
 */

#ifndef DweetChannel_h
#define DweetChannel_h

//
// Each MenloDweet instance has its own independent receive
// and transmit buffers as well as internal processing states
// in both MenloDweet common support and NMEA0183 transport. As
// such each MenloDweet instance is bound to a transport channel
// such as serial, radioserial, HTTP, TCP, COAP, etc.
//
// This allows each instance to act independently when receiving
// and processing MenloDweet requests as they may arrive as
// fragments, etc.
//
// This base class allows a uniform contract and type.
//

class DweetChannel : public MenloDweet {

public:

    DweetChannel() {
    }

    void reset() {
      m_nmea.reset();
    }

    //
    // This allow access to underying NMEA routines without
    // having to wrap them.
    //
    MenloNMEA0183* getNMEA() {
        return &m_nmea;
    }

protected:

    // m_nmea instance holds the output buffer
    MenloNMEA0183 m_nmea;

private:

};

#endif // DweetChannel_h
