
# DOTS
Distributed Objects in Time and Space

Distributed Objects - Die Kommunikation wird über verteilte Daten-Objekte realisiert.
Time - Publisher und Subscriber müssen nicht zeitgleich verbunden sein (Zeitlich entkoppelt)
Space - Es ist möglich, die Datenmodelle in getrennten Bereichen (Spaces) zu organisieren.

## Übersicht
DOTS ist ein publish/subscribe Interprozess-Kommunikationssystem, dass Objekte austauscht, bzw. verteilt.

TODO: HIER DIE ROLLE DES SERVERS ERKLÄREN: SERVER SELBST HAT KEINE INITIATIVE, DIENT NUR ALS MITTLER UND CACHE

Für die Kommunikation zwischen den Clients wird in der DOTS-Beschreibungssprache ein Typ definiert, Objekte können
dann als Instanzen von diesem Typ übertragen werden.
In DOTS-Typen können Felder als Schlüssel definiert werden, für Objekte mit definiertem Schlüssel kann mehr als
eine Instanz angelegt werden.
DOTS-Objekte sind reine Datenobjekte und es gibt keine Funktionsaufrufe (RPC).

Typischerweise werden die Objekte im Server gespeichert, so dass es für einen Subscriber nicht erforderlich ist,
vor dem Zeitpunkt des publizierens verbunden und subscribed zu sein.

### Client-Arten

Es gibt prinzipell zwei Arten von DOTS-Clients: statische und dynamische (Mischungen sind möglich).

Statische Clients sind Clients, die die DOTS-Typen, die sie verwenden einkompiliert haben, d.h. sie verwenden für
diese Typen nur ihre eigenen Deskriptoren.

Dynamische Clients haben keine DOTS-Typ-Definitonen einkompiliert und verarbeiten die von den anderen Clients gesendeten
Struktur-Deskriptoren.

## Technologie
DOTS-Clients verbinden sich über TCP oder UNIX-Streaming-Socket (Linux-only) mit dem Server und tauschen die Objekte aus,
die mit CBOR serialisiert sind.

### Verbindungsarten
Mindestens die C++-Implementierung beherrscht das "early-subscribe"-Feature. Dabei werden während des
Verbindungsaufbaus zum Server alle Deskriptoren gesendet und alle Typen subscribed, die zur Laufzeit des
Programms verwendet werden. Die Benutzer-Programmlogik startet erst dann, wenn die Container
des Clients schon mit allen Daten gefüllt sind.

Für dieses Feature muss bei Verbindungsaufbau in DotsMsgConnect das Feld preloadCache auf true gesetzt
werden. Der Client befindet sich dann im "early_subscribe"-Zustand. Nachdem der Client alle Subscribes
durchgeführt hat, schickt er DotsMsgConnect mit preloadClientFinished=true. Der Server beendet den
"early_subscribe"-Zustand, in dem er DotsMsgConnectResponse mit preloadFinished=true an den Client
sendet.

### Objekt-Transport

### TCP

Objekte werden über TCP verschickt, in dem für jedes Objekt die Länge des DotsTransportHeaders in Byte, der DotsTransportHeader
in CBOR-Serialisierter Form und schließlich, das ebenfalls in CBOR serialisierte Objekt übertragen wird.

| Header-Länge (uint16) | DotsTransportHeader | DOTS-Objekt |
| ----- | ----- | ----- |


#### Algorithmus zum Einlesen eines DOTS-Objekts
1. Lese uint16 als Header-Size
2. Lese für 'Header-Size'
3. Deserialisiere DotsTransportHeader
4. Lese für 'Payload-Länge' aus DotsTransportHeader als Payload
5. Deserialisiere 'DOTS-Objekt' in den im DotsTransportHeader angegebenen Typ.

### Cache
Beschreibung des Cache

### Container
Ein Container kann Instanzen eines bestimmten DOTS-Typs speichern. Für jede Instanz werden noch
Metainformationen gehalten:

* lastOperation - Letze Operation (create, update, remove)
* lastUpdateFrom - Von welchem Client erfolgte die letzt Veränderung
* modified - Zeitpunkt der letzen Veränderung
* createdFrom - Welcher Client hat das Objekt angelegt
* created - Zeitpunkt der Erstellung des Objekts
* localUpdateTime - Zeitpunkt, an dem der Container aktualisiert wurde.

#### Create, Update, Remove

#### Merge
Wird ein bestehendes Objekt in einem Container aktualisiert, so werden die gülten Eigenschaften
des neuen Objekts im Container überschrieben. Eigenschaften des Objekts im Container, die gültig sind,
aber nicht von dem neuen Objekt überschrieben werden, bleiben bestehen.

#### Callbackdata / Metadaten
An Containern können Callbacks installiert werden, die für jede Aktualisierung des
Containers aufgerufen werden.
Der Callback enthält neben der Referenz auf das Objekt auch noch die Metadaten zu dem Objekt (und Update).

Bei Erstellung und Aktualisierung des Objekts, wird zuerst der Container verändert und dann der Callback
aufgerufen.

Bei Entfernen des Objekts, wird zuerst der Callback aufgerufen, und dann erst das Objekt aus dem Container
entfernt.

## DOTS-Typen und Objekte

DOTS-Strukturen sind Strukturen, die aus primitven Typen oder anderen DOTS-Typen bestehen.
Für jedes Feld einer DOTS-Struktur muss ein DOTS-Tag definiert werden. Ein DOTS-Tag ist eine wichtige Eigenschaft einer
DOTS-Struktur; das DOTS-Tag wird für die Übertragung über das Netzwerk verwendet. Ein DOTS-Tag sollte nach Verwendung
nicht mehr von der semantischen Bedeutung des Feldes getrennt werden. Die Verwendung von DOTS-Tags für die Datenserialierung
reduziert die benötigte Netzwerkbandbreite, beschleunigt die Verarbeitung und eröffnet die Möglichkeit, die Namen der
Felder nachträglich anzupassen, ohne die Verarbeitung der Daten in existierenden Clients zu beschädigen.

### Primitive Typen
| DOTS-Typ  | CBOR-Representation           | Beschreibung |
| --------- | --------------------- | ------------ |
| int8      | if < 0 Type 1         | 8-Bit signed integer |
| int8      | else Type 0           | 8-Bit signed integer |
| int16     | ""                    | 8-Bit signed integer |
| int32     | ""                    | 16-Bit signed integer |
| int64     | ""                    | 32-Bit signed integer |
|           |                       |                         |
| uint8     | Type 0                | 64-Bit unsigned integer |
| uint16    | ""                    | 16-Bit unsigned integer |
| uint32    | ""                    | 32-Bit unsigned integer |
| uint64    | ""                    | 64-Bit unsigned integer |
|           |                       |                         |
| float16   | Type 7.25             | 16-Bit floating-point |
| float32   | Type 7.26             | 32-Bit floating-point |
| float64   | Type 7.27             | 64-Bit floating-point |
|           |                       |                         |
| bool      | Type 7.20 / 7.21      | Boolean |
| string    | Type 3                | String für Text (UTF-8) |
| bytes     | Type 2                | Binärdaten |
|           |                       |                         |
| timepoint | Type 7                | Epoch, Zeit in Sekunden mit 6 Nachkommastellen (Übertragung als float64) |
| steady_timepoint | Type 7         | Client-Spezifischer Zeitpunkt in Sekunden, Zeit in Sekunden | sbyte |
| duration  | Type 7                | Zeitdauer in Sekunden TODO: WIE VIELE NACHKOMMASTELLEN, WIE ÜBERTRAGEN |
| uuid      | Type 2 size=16        | UUID nach RFC 4122 (Binär 16 Byte) |
| vector<>  | TODO:                 | TODO:

## DOTS Type Description Language

Definition eines Enums:

    enum <Enum-Name> [<Eigenschaften> {
        <Tag>: <Enum-Element-Name> = <Enum-Value>
    }

Definition einer Struktur:

    struct <Strukturname> [<Eigenschaften>] {
        <Tag>: [<Feld-Eigenschaften>] <Dots-Typ|Enum|Struktur> <Feldnamne>;
    }

Beispiel:

    struct Unterstruktur {
        1: bool a;
        2: int8 b;
    }

    struct BeispielTyp {
        1: [key] uuid id;
        2: string name;
        3: bool someBoolean;
        4: Unterstruktur sub;
    }`

TODO: TYP 'VECTOR' BESCHREIBEN

### Eigenschaften von DOTS-Typen
* cached - DOTS-Objekte werden im Server im Cache temporär gespeichert. Objekte ohne Cached Attribut werden nur an aktive subscribed Clients verteilt und sind danach nicht mehr verfügbar.
* cleanup - DOTS-Objekte werden automatisch aus dem Cache entfernt, wenn die Verbindung des erzeugenden Clients zum Server
beendet wird.
* substruct_only - Der DOTS-Typ kann nur als Untertyp/Struktur in anderen DOTS-Typen genutzt werden. Ein direktes subscriben oder publishen ist nicht möglich.
* internal - DOTS-interne DOTS-Typen sind als "internal" markiert.
* persistent - Die Objekte dieses DOTS-Typen sollen von einer Persistierungsschicht persistiert werden.

### Eigenschaften von Feldern von DOTS-Typen
* key - Das Feld ist Teil des Schlüssels
* ctor - (Sprachspezifisch) Das Feld soll im Konstrutor übergeben werden
* deprecated - Das Feld wird in naher Zukunft entfernt und sollte nicht mehr verwendet werden.
* removed - Das Feld wird vom Code-Generator nicht mehr angelegt. Die Eigenschaft dient, dazu, doppelte
DOTS-Tags für den entsprechenden DOTS-Typ zu vermeiden.

## Server-Verbindung

Der DOTS Daemon hört auf TCP-Port 11234 (und Unix-Domain-Socket /var/run/dotsd_<pid>).

UNIX Domain Socket:
Ist nur ein Socket verfügbar, wird dieser automatisch genutzt. Sind mehrere verfügbar, muss das Anwendungsprogramm
den Socket angeben.

### Verbindungsaufbau

    Client                                           Server
    --|                                                 |
      | ------------- Connect via TCP ---------------->B|
      |B<------------ Send DotsMsgHello ---------------B|
    1 |B                                                |
      |B------------ Send DotsMsgConnect -------------> |
      |                                                 | -> Check Credentials
      |                                                 | <- Response
    --|B<----------- DotsMsgConnectResponse ----------- | -> Snapshot Cache?
      |B                                                |
      |B------------- Pub/Sub Descriptors ------------> | # Übertragung der bekannten Typdeskriptoren
      |B-------------- Subscriptions -----------------> |
      |B-------------- Subscriptions -----------------> |
      |B-------------- Subscriptions -----------------> |
    2 |B------- DotsMsgConnect (preload.Fin.) --------> |
      |                                                 |
      | <-------- Subscribed Content from Cache ------- |
      | <-------- Subscribed Content from Cache ------- |
      | <-- DotsMsgConnectResponse (preloadClientFin.)- |
    --|                                                 |
      |                                                 | <- Send Updates realtime
    3 |                                                 |
      |                                                 |


    States:
    1 - Connecting
    2 - Early subscribe
    3 - Connected

### Subscribe
Um einen Typen/Gruppe zu subscriben, schickt der Client ein DotsMember-Objekt, in dem
der Gruppen-Name (groupName) und das DotsMemberEvent auf "join" gesetzt ist.

Der Server schickt zuerst den Inhalt des Caches, gefolgt von einem
DotsCacheInfo{typeName:<name>, endTransmission:true}.

    Client                                           Server
      |                                                 |
      | ------------- Send DotsMember ---------------->B| # Legt den gesamten Container des angeforderten Typs
      |<------------- Send Data ------------------------| # in den Ausgangspuffer
      |<------------- Send Data ------------------------|
      |<------------- Send Data ------------------------|
      |<------------- Send CacheEnd --------------------| # CacheEnd teilt dem Client mit, dass keine weiteren
      |                                                 | # Daten im Container sind.


### Unsubscribe
Um eine Gruppe zu verlassen (unsubscribe), schickt der Client ein DotsMember-Objekt.
DotsMember{groupName:<name>, event:leave}.

## Introspection

Für jeden DOTS-Typen wird zur Laufzeit ein StructDescriptor-Objekt erzeugt und an den Server gesendet, bevor eine Instanz des DOTS-Typen gepublished oder subscribed wird. Ausgenommen sind die internen DOTS-Typen (Attribut [internal]), die dem Server bereits bekannt sein müssen.
Ein StructDescriptor-Objekt behinhaltet die Struktur eines beliebigen DOTS-Typen.
Für den Zwischenspeicherfunktion des DOTS Daemons (Cache) ist der StructDescriptor wichtig, da dieser erst dem Server
ermöglicht, die Objekte des Typen entsprechend in dem Typen-Container des Server abzulegen.

## Interne DOTS-Typen

### Server-Verbindung

#### DotsHeader
    struct DotsHeader [internal,cached=false] {
        1: string typeName; // name of the type, contained in the payload.
        2: time_point sentTime; // absolute time point when the originating client sends this update.
        7: time_point serverSentTime; // absolute time point when the server sends the this update.
        3: property_set attributes; // value properties that are contained in the payload.
        4: bool removeObj; // true of the contained object should be removed.
        5: uint32 sender; // originating sender of this update.
        6: bool isFromMyself; // is set to true in the client's callback when the sender-name matches the specific client name.
    }

#### DotsTransportHeader
    struct DotsTransportHeader [internal,cached=false] {
        1: string nameSpace;
        2: string destinationGroup;
        3: DotsHeader dotsHeader;
        4: uint32 payloadSize;
        5: uint32 destinationClientId;
    }

#### DotsMsgHello
    struct DotsMsgHello [internal,cached=false] {
        1: string serverName;
        2: uint64 authChallenge;
    }

#### DotsMsgConnect
    // Used in two cases:
    //
    // 1. After DotsMsgHello the client sends DotsMsgConnect with it's name
    // and preloadCache set to true, if the client want's to preload it's cache.
    //
    // 2. In case of preloading, the client needs to tell the server, when all subscriptions
    // has been sent. This is done by sending DotsMsgConnect with preloadClientFinished set to true.
    struct DotsMsgConnect [internal,cached=false] {
        1: string clientName; // transmit the name of the client to the server.
        2: bool preloadCache; // set to true, when the client wants to preload it's cache.
        3: bool preloadClientFinished; // transmit and set to true, when the client has send all subscriptions for preloading.
    }

#### DotsMsgConnectResponse
    struct DotsMsgConnectResponse [internal,cached=false] {
        1: string serverName;
        5: uint32 clientId;
        2: bool accepted;
        3: bool preload;
        4: bool preloadFinished;
    }

### Server-Aktionen

#### DotsClearCache
    // Clears (removes) the objects in the container of the listed types.
    struct DotsClearCache [internal,cached=false] {
        1: vector<string> typeNames; // names of the types to clear
    }

#### DotsMember
    // With DotsMember, a client can join or leave groups.
    struct DotsMember [internal,cached=false] {
        1: string groupName; // group to join or leave
        2: DotsMemberEvent event; // set to join or leave
        3: uint32 client; // ID of the client that join or leave.
    }

#### DotsMemberEvent
    enum DotsMemberEvent {
        1: join,
        2: leave,
        3: kill
    }

#### DotsCacheInfo
    struct DotsCacheInfo [internal,cached=false] {
        1: string typeName;
        2: bool startTransmission;
        3: bool endTransmission;
        4: bool endDescriptorRequest;
    }

#### DotsDescriptorRequest
    struct DotsDescriptorRequest [internal,cached=false] {
        1: vector<string> whitelist;
        2: vector<string> blacklist;
    }

### Server-Informationen

#### DotsClient
    struct DotsClient [internal,cleanup=true] {
        1: [key] uint32 id;
        2: string name;
        3: bool running;
        4: vector<string> publishedTypes;
        5: vector<string> subscribedTypes;
    }
#### DotsStatistics
    struct DotsStatistics [internal] {
        1: uint64 bytes;
        2: uint64 packages;
    }

#### DotsCacheStatus
    struct DotsCacheStatus [internal] {
        1: uint32 nrTypes;
        2: uint64 size;
    }

#### DotsResourceUsage
    struct DotsResourceUsage [internal] {
        1: int32 minorFaults; // number of minor page-faults
        2: int32 majorFaults; // number of major page-faults
        3: int32 inBlock;
        4: int32 outBlock;
        5: int32 nrSignals; // number of received signals
        6: int32 nrSwaps; // number of memory swaps
        7: int32 nrVoluntaryContextSwitches;
        8: int32 nrInvoluntaryContextSwitches;
        9: int32 maxRss; // maximum used memory (peak) in Kb over process lifetime
        10: duration userCpuTime; // used 'user' CPU-time in seconds
        11: duration systemCpuTime; // used 'system' CPU-time in seconds
    }

#### DotsDaemonStatus
    struct DotsDaemonStatus [internal] {
        1: [key] string serverName;
        2: time_point startTime;
        3: DotsStatistics received;
        4: DotsStatistics sent;
        5: DotsCacheStatus cache;
        6: DotsResourceUsage resourceUsage;
    }

#### DotsMsgError
    struct DotsMsgError [internal,cached=false] {
        1: int32 errorCode;
        2: string errorText;
    }


### Typ-Deskriptoren

#### StructDescriptorData

#### DotsStructFlags
    struct DotsStructFlags [internal,cached=false] {
        1: bool cached; // if the struct is cached by the server (or generally in the containers).
        2: bool internal; // if the struct is an DOTS-internal type.
        3: bool persistent; // if the struct should be saved to non-volatile memory.
        4: bool cleanup; // if an object of this struct should be removed, if the creator disconnectes from the server.
        5: bool local;
        6: bool substructOnly; // when a struct is marked as 'substruct_only', it cannot be published directly.
    }

#### DotsStructScope
    enum DotsStructScope {
        1: program, // Ony for client-internal use
        2: server, // Will only be routed to clients, connected to the same server
        3: site,   // Route to clients within the same site
        4: global  // No limitation
    }

#### EnumDescriptorData
    // Descriptor for enumerations
    struct EnumDescriptorData [internal,cached=false] {
        1: [key] string name;
        2: vector<EnumElementDescriptor> elements;
    }

#### EnumElementDescriptor
    struct EnumElementDescriptor [internal,cached=false] {
        1: int32 enum_value;
        2: string name;
        3: uint32 tag;
    }

#### StructPropertyData
    struct StructPropertyData [internal,cached=false] {
        1: string name; // name of attribute
        2: uint32 tag; // unique attribute tag
        3: bool isKey; // is this property a key or keypart
        4: string type; // name of type... or
        5: uint32 typeId; //... id of type
    }

#### StructDescriptorData
    struct StructDescriptorData [internal,cached=false] {
        1: [key] string name; // name of the struct
        2: vector<StructPropertyData> properties; // list of struct-properties
        3: StructDocumentation documentation; // struct documentation
        4: DotsStructScope scope; // scope for which the struct is valid
        5: DotsStructFlags flags; // struct flags
    }

#### StructDocumentation
    struct StructDocumentation [internal,cached=false] {
        1: string description;
        2: string comment;
    }

### DOTS Utilities

#### DotsConnectionState
    enum DotsConnectionState {
       1: connecting,
       2: early_subscribe,
       3: connected,
       4: suspended,
       5: closed
    }



#### DotsRecordHeader
    struct DotsRecordHeader [internal,cached=false] {
        1: [key] string name;
        2: time_point startTime;
        3: vector<string> whitelist;
    }



## Glossar

* DOTS-Objekt - Konkretes Objekte / Instanz eines DOTS-Typen
* DOTS-Typ - Ein von DOTS verwendeter Datentyp, u.a. DOTS-Struktur und DOTS-Enum
* DOTS-Struktur - Definition einer DOTS-Struktur, Grundlage für DOTS-Objekte
* DOTS-Enum - Definition einer Enumeration
* DOTS-Tag - Ganzzahl zwischen 1 und 31, für die Erhaltung der Serialisierungs-Kompatibilität.
