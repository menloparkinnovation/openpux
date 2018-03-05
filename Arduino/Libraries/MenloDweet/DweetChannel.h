
/*
 * Copyright (C) 2016 Menlo Park Innovation LLC
 *
 * This is licensed software, all rights as to the software
 * is reserved by Menlo Park Innovation LLC.
 *
 * A license included with the distribution provides certain limited
 * rights to a given distribution of the work.
 *
 * This distribution includes a copy of the license agreement and must be
 * provided along with any further distribution or copy thereof.
 *
 * If this license is missing, or you wish to license under different
 * terms please contact:
 *
 * menloparkinnovation.com
 * menloparkinnovation@gmail.com
 */

/*
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
