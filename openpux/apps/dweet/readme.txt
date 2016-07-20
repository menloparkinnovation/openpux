
04/28/2016

This will contain the main dweet dispatcher for Openpux.

All dweets from all transports arrive here.

It raises events for interested listeners.

All senders/response of dweets are through here as well.

It will have a context to contain information about the transport, device
id, etc.

This will allow the serial dweet gateway to operate with just a simple
serial handler and seamlessly integrate across transports.



