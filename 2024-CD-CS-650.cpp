#include <iostream>
#include <string>

using namespace std;

// Utility function to pause the console
void pause()
{
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

// --- NODE STRUCTURES ---

// Node for the Parcel History Linked List
// Used to store audit logs (e.g., "Arrived at Hub", "Delivered")
class HistoryNode
{
public:
    string event;
    HistoryNode* next;

    HistoryNode(string e)
    {
        event = e;
        next = nullptr;
    }
};

// --- CORE ENTITY ---

// Represents a single package in the system
class Parcel
{
private:
    string id;
    int priority;           // 1=High (Overnight), 3=Low (Standard)
    double weight;
    string weightCat;       // Auto-calculated: Light/Medium/Heavy
    string destination;
    string zone;            // Auto-calculated based on city
    string status;          // Current state (e.g., "At Hub", "In Transit")
    int deliveryAttempts;   // Track failed attempts
    bool isMissing;         // Flag for lost parcels
    int assignedRiderId;    // Links parcel to a specific rider (Fix for load management)

    // Linked List Head/Tail for history logs
    HistoryNode* historyHead;
    HistoryNode* historyTail;

    // Helper logic to categorize weight
    string determineWeightCat(double w)
    {
        if (w < 5.0) return "Light";
        else if (w < 20.0) return "Medium";
        return "Heavy";
    }

    // Helper logic to map cities to geographic zones
    string determineZone(string city)
    {
        if (city == "Islamabad" || city == "Peshawar") return "North";
        if (city == "Lahore" || city == "Faisalabad") return "Central";
        if (city == "Karachi" || city == "Multan") return "South";
        return "Unknown";
    }

public:
    Parcel() {}
    Parcel(string pid, int p, double w, string dest)
    {
        id = pid;
        priority = p;
        weight = w;
        weightCat = determineWeightCat(w);
        destination = dest;
        zone = determineZone(dest);
        status = "At Hub";
        deliveryAttempts = 0;
        isMissing = false;
        assignedRiderId = -1; // -1 indicates no rider assigned
        historyHead = nullptr;
        historyTail = nullptr;
        addEvent("Parcel Received at Hub (Lahore)");
    }

    // Getters
    string getID() { return id; }
    int getPriority() { return priority; }
    double getWeight() { return weight; }
    string getDest() { return destination; }
    string getStatus() { return status; }
    string getZone() { return zone; }
    string getWeightCat() { return weightCat; }
    bool getMissingStatus() { return isMissing; }

    // Rider Assignment (Crucial for load tracking)
    void setRiderId(int rid)
    {
        assignedRiderId = rid;
    }
    int getRiderId()
    {
        return assignedRiderId;
    }
    // Status Management
    void setStatus(string s)
    {
        status = s;
    }
    void markMissing(bool flag)
    {
        isMissing = flag;
    }

    // Appends a new event to the History Linked List
    void addEvent(string desc)
    {
        HistoryNode* newNode = new HistoryNode(desc);
        if (historyHead == nullptr)
        {
            historyHead = newNode;
            historyTail = newNode;
        }
        else
        {
            historyTail->next = newNode;
            historyTail = newNode;
        }
    }

    void incrementAttempts()
    {
        deliveryAttempts++;
    }
    int getAttempts()
    {
        return deliveryAttempts;
    }

    // Compact display for list views
    void printRow()
    {
        cout << " > ID: " << id << " | Priority: " << priority << " | Weight: " << weight << "kg (" << weightCat << ")" << " | Destination: " << destination << " | Zone: " << zone << endl;
    }

    // Full detailed view including history log
    void printDetails()
    {
        cout << "\n--- Parcel " << id << " Details ---" << endl;
        cout << "Priority: " << priority << " | Weight: " << weight << "kg (" << weightCat << ")" << endl;
        cout << "Zone: " << zone << " | Destination: " << destination << endl;
        cout << "Current Status: " << status << endl;
        if (assignedRiderId != -1)
        {
            cout << "Assigned Rider ID: " << assignedRiderId << endl;
        }
        if (isMissing)
        {
            cout << "ALERT: PARCEL FLAGGED AS MISSING!" << endl;
        }
        cout << "Delivery Attempts: " << deliveryAttempts << endl;
        cout << "\n--- History Log ---" << endl;

        // Traverse History Linked List
        HistoryNode* temp = historyHead;
        while (temp != nullptr)
        {
            cout << " >> " << temp->event << endl;
            temp = temp->next;
        }
        cout << "---------------------------------" << endl;
    }
};

// --- DATA STRUCTURE NODES (CUSTOM IMPLEMENTATION) ---

// Node for Queue
class QueueNode
{
public:
    Parcel* data;
    QueueNode* next;
    QueueNode(Parcel* p)
    {
        data = p;
        next = nullptr;
    }
};

// Snapshot state for Undo functionality
struct UndoState
{
    Parcel* p;
    string prevStatus;
    string description;
};

// Node for Stack
class StackNode
{
public:
    UndoState state;
    StackNode* next;
    StackNode(UndoState s)
    {
        state = s;
        next = nullptr;
    }
};

// Node for Graph Adjacency List
class RoadConnections
{
public:
    int destination;
    int weight;
    bool isBlocked; // Handles road blockage requirement
    RoadConnections* next;

    RoadConnections(int dest, int w)
    {
        destination = dest;
        weight = w;
        isBlocked = false;
        next = nullptr;
    }
};

// --- CUSTOM DATA STRUCTURES ---

/*
    Module: Queues (Pickup, Warehouse, Transit)
    Implementation: Linked List (FIFO)
*/
class ParcelQueue
{
private:
    QueueNode* front;
    QueueNode* rear;
public:
    ParcelQueue()
    {
        front = rear = nullptr;
    }

    // O(1) Enqueue
    void enqueue(Parcel* p)
    {
        QueueNode* temp = new QueueNode(p);
        if (rear == nullptr)
        {
            front = rear = temp;
            return;
        }
        rear->next = temp;
        rear = temp;
    }

    // O(1) Dequeue
    Parcel* dequeue()
    {
        if (front == nullptr)
        {
            return nullptr;
        }
        QueueNode* temp = front;
        Parcel* p = front->data;
        front = front->next;
        if (front == nullptr)
        {
            rear = nullptr;
        }
        delete temp;
        return p;
    }

    bool isEmpty()
    {
        return front == nullptr;
    }

    // Iterates list to show contents without removing
    void displayContent()
    {
        if (front == nullptr)
        {
            cout << "  (Queue is empty)" << endl;
            return;
        }
        cout << "\n[ WAITING IN QUEUE ]" << endl;
        QueueNode* temp = front;
        while (temp != nullptr)
        {
            temp->data->printRow();
            temp = temp->next;
        }
        cout << "--------------------" << endl;
    }
};

/*
    Module: Undo/Replay
    Implementation: Stack using Linked List (LIFO)
*/
class UndoStack
{
private:
    StackNode* top;
public:
    UndoStack()
    {
        top = nullptr;
    }

    void push(Parcel* p, string prevStatus, string desc)
    {
        UndoState s = { p, prevStatus, desc };
        StackNode* newNode = new StackNode(s);
        newNode->next = top;
        top = newNode;
    }

    bool pop(UndoState& retState)
    {
        if (top == nullptr) return false;
        StackNode* temp = top;
        retState = top->state;
        top = top->next;
        delete temp;
        return true;
    }

    bool isEmpty()
    {
        return top == nullptr;
    }
};

// Entity for Rider/Driver
class Rider
{
public:
    int id;
    string name;
    double capacity;    // Max weight capacity
    double currentLoad; // Current active load

    Rider() {}
    Rider(int rid, string rname, double cap)
    {
        id = rid;
        name = rname;
        capacity = cap;
        currentLoad = 0;
    }

    // Logic: Only assign if weight fits in remaining capacity
    bool assignParcel(Parcel* p)
    {
        if (currentLoad + p->getWeight() <= capacity)
        {
            currentLoad += p->getWeight();
            return true;
        }
        return false;
    }
};

/*
    Module: Sorting Engine
    Implementation: Min-Heap
    Logic: Sorts primarily by Priority (1 is high), secondarily by Weight (Heavier first)
*/
class PriorityScheduler
{
private:
    Parcel* heapArray[100]; // Fixed size array for heap
    int currentSize;

    void swap(int a, int b)
    {
        Parcel* temp = heapArray[a];
        heapArray[a] = heapArray[b];
        heapArray[b] = temp;
    }

    bool isHigherPriority(Parcel* a, Parcel* b)
    {
        if (a->getPriority() != b->getPriority())
            return a->getPriority() < b->getPriority();
        return a->getWeightCat() == "Heavy" && b->getWeightCat() != "Heavy"; // Heavy gets preference if priorities equal
    }

    void heapifyUp(int index)
    {
        if (index <= 0) return;
        int parent = (index - 1) / 2;
        if (isHigherPriority(heapArray[index], heapArray[parent]))
        {
            swap(index, parent);
            heapifyUp(parent);
        }
    }

    void heapifyDown(int index)
    {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < currentSize && isHigherPriority(heapArray[left], heapArray[smallest]))
        {
            smallest = left;
        }
        if (right < currentSize && isHigherPriority(heapArray[right], heapArray[smallest]))
        {
            smallest = right;
        }

        if (smallest != index)
        {
            swap(index, smallest);
            heapifyDown(smallest);
        }
    }

public:
    PriorityScheduler()
    {
        currentSize = 0;
    }

    void insert(Parcel* p)
    {
        if (currentSize < 100)
        {
            heapArray[currentSize] = p;
            heapifyUp(currentSize);
            currentSize++;
        }
    }

    Parcel* extractMin()
    {
        if (currentSize <= 0)
        {
            return nullptr;
        }
        Parcel* root = heapArray[0];
        heapArray[0] = heapArray[currentSize - 1];
        currentSize--;
        heapifyDown(0);
        return root;
    }

    bool isEmpty()
    {
        return currentSize == 0;
    }

    void displayContent()
    {
        if (currentSize == 0)
        {
            cout << "  (No parcels in sorting queue)" << endl;
            return;
        }
        cout << "\n[ WAITING PARCELS IN SORTING QUEUE ]" << endl;
        for (int i = 0; i < currentSize; i++)
        {
            heapArray[i]->printRow();
        }
        cout << "------------------------------------" << endl;
    }
};

/*
    Module: Routing
    Implementation: Weighted Graph (Adjacency List)
    Algorithms: Dijkstra (Shortest Path), DFS (All Paths)
*/
class RoutingGraph
{
private:
    static const int MAX_CITIES = 10;
    string cityNames[MAX_CITIES];
    RoadConnections* adjList[MAX_CITIES];
    int numCities;

    int getCityIndex(string name)
    {
        for (int i = 0; i < numCities; i++) if (cityNames[i] == name) return i;
        return -1;
    }

    void printAllPathsUtil(int u, int d, bool visited[], int path[], int& pathIdx)
    {
        visited[u] = true;
        path[pathIdx] = u;
        pathIdx++;

        if (u == d)
        {
            cout << "Route Option: ";
            for (int i = 0; i < pathIdx; i++)
            {
                cout << cityNames[path[i]];
                if (i < pathIdx - 1) cout << " -> ";
            }
            cout << endl;
        }
        else
        {
            RoadConnections* temp = adjList[u];
            while (temp != nullptr)
            {
                // Check if road is blocked before traversing
                if (!visited[temp->destination] && !temp->isBlocked)
                {
                    printAllPathsUtil(temp->destination, d, visited, path, pathIdx);
                }
                temp = temp->next;
            }
        }
        pathIdx--;
        visited[u] = false;
    }

public:
    RoutingGraph()
    {
        numCities = 0;
        for (int i = 0; i < MAX_CITIES; i++)
        {
            adjList[i] = nullptr;
        }
    }

    void addCity(string name)
    {
        if (numCities < MAX_CITIES)
        {
            cityNames[numCities++] = name;
        }
    }

    void addRoute(string src, string dest, int weight)
    {
        int u = getCityIndex(src);
        int v = getCityIndex(dest);
        if (u != -1 && v != -1)
        {
            // Undirected Graph Logic
            RoadConnections* newNode = new RoadConnections(v, weight);
            newNode->next = adjList[u];
            adjList[u] = newNode;

            newNode = new RoadConnections(u, weight);
            newNode->next = adjList[v];
            adjList[v] = newNode;
        }
    }

    // Dynamic update for road blocks
    void blockRoad(string src, string dest, bool status)
    {
        int u = getCityIndex(src);
        int v = getCityIndex(dest);
        if (u == -1 || v == -1) return;

        RoadConnections* temp = adjList[u];
        while (temp != nullptr)
        {
            if (temp->destination == v)
            {
                temp->isBlocked = status;
                break;
            }
            temp = temp->next;
        }
        temp = adjList[v];
        while (temp != nullptr)
        {
            if (temp->destination == u)
            {
                temp->isBlocked = status;
                break;
            }
            temp = temp->next;
        }
        cout << (status ? "Road Blocked: " : "Road Restored: ") << src << " <--> " << dest << endl;
    }

    // Algorithm: Dijkstra's Shortest Path
    void findShortestPath(string startCity, string endCity)
    {
        int start = getCityIndex(startCity);
        int end = getCityIndex(endCity);
        if (start == -1 || end == -1)
        {
            cout << "Invalid Cities" << endl;
            return;
        }
        int dist[MAX_CITIES];
        int parent[MAX_CITIES];
        bool visited[MAX_CITIES];

        for (int i = 0; i < numCities; i++)
        {
            dist[i] = INT_MAX;
            visited[i] = false;
            parent[i] = -1;
        }
        dist[start] = 0;

        for (int count = 0; count < numCities - 1; count++)
        {
            int minVal = INT_MAX, u = -1;
            for (int i = 0; i < numCities; i++)
            {
                if (!visited[i] && dist[i] <= minVal)
                {
                    minVal = dist[i];
                    u = i;
                }
            }
            if (u == -1)
            {
                break;
            }
            visited[u] = true;

            RoadConnections* temp = adjList[u];
            while (temp != nullptr)
            {
                if (!temp->isBlocked && !visited[temp->destination] && dist[u] != INT_MAX && dist[u] + temp->weight < dist[temp->destination])
                {
                    dist[temp->destination] = dist[u] + temp->weight;
                    parent[temp->destination] = u;
                }
                temp = temp->next;
            }
        }

        if (dist[end] == INT_MAX)
        {
            cout << "ALERT: No valid path exists (Roads might be blocked)!" << endl;
        }
        else
        {
            cout << "Optimal Route (Cost: " << dist[end] << "): ";
            printPathRecursive(parent, end);
            cout << endl;
        }
    }

    void printPathRecursive(int parent[], int j)
    {
        if (parent[j] == -1)
        {
            cout << cityNames[j];
            return;
        }
        printPathRecursive(parent, parent[j]);
        cout << " -> " << cityNames[j];
    }

    // Algorithm: DFS to find all paths
    void findAllRoutes(string src, string dest)
    {
        int s = getCityIndex(src);
        int d = getCityIndex(dest);
        bool visited[MAX_CITIES] = { false };
        int path[MAX_CITIES];
        int pathIdx = 0;

        cout << "Calculating all viable alternative routes..." << endl;
        printAllPathsUtil(s, d, visited, path, pathIdx);
    }
};

/*
    Module: Tracking
    Implementation: Hash Table (Chaining method for collision resolution)
*/
class TrackerTable
{
private:
    struct HashNode
    {
        Parcel* p;
        HashNode* next;
    };
    HashNode* table[50]; // Table size 50

    // Simple hash function (Sum of ASCII char values % Size)
    int hashFunc(string id)
    {
        int sum = 0;
        for (char c : id) sum += c;
        return sum % 50;
    }

public:
    TrackerTable()
    {
        for (int i = 0; i < 50; i++)
        {
            table[i] = nullptr;
        }
    }

    void insert(Parcel* p)
    {
        int idx = hashFunc(p->getID());
        HashNode* newNode = new HashNode{ p, table[idx] };
        table[idx] = newNode;
    }

    // O(1) Search (Average Case)
    Parcel* search(string id)
    {
        int idx = hashFunc(id);
        HashNode* temp = table[idx];
        while (temp != nullptr)
        {
            if (temp->p->getID() == id)
            {
                return temp->p;
            }
            temp = temp->next;
        }
        return nullptr;
    }
};

// --- CONTROLLER CLASS ---
class CourierSystem
{
private:
    // Operational Queues
    ParcelQueue pickupQueue;
    PriorityScheduler sortingEngine;
    ParcelQueue warehouseQueue;
    ParcelQueue transitQueue;
    // Modules
    RoutingGraph routingEngine;
    TrackerTable trackingEngine;
    UndoStack undo;
    Rider riders[3];

public:
    CourierSystem()
    {
        // Initialize Map
        routingEngine.addCity("Lahore");
        routingEngine.addCity("Islamabad");
        routingEngine.addCity("Karachi");
        routingEngine.addCity("Multan");
        routingEngine.addCity("Peshawar");

        routingEngine.addRoute("Lahore", "Islamabad", 380);
        routingEngine.addRoute("Lahore", "Multan", 340);
        routingEngine.addRoute("Islamabad", "Peshawar", 180);
        routingEngine.addRoute("Multan", "Karachi", 950);
        routingEngine.addRoute("Lahore", "Karachi", 1200);
        routingEngine.addRoute("Lahore", "Peshawar", 560);

        // Initialize Riders with different capacities
        riders[0] = Rider(1, "Ali (Bike)", 10.0);
        riders[1] = Rider(2, "Bob (Van)", 50.0);
        riders[2] = Rider(3, "Charlie (Truck)", 200.0);
    }

    // Option 1: New Parcel Entry
    void registerParcel(string id, int prio, double w, string dest)
    {
        Parcel* p = new Parcel(id, prio, w, dest);
        trackingEngine.insert(p); // Add to tracking system

        pickupQueue.enqueue(p);   // Add to first workflow stage
        p->setStatus("In Pickup Queue");
        undo.push(p, "Created", "Added to Pickup");

        cout << "Parcel registered and added to Pickup Queue." << endl;
    }

    // Option 2: Move from Pickup -> Sorting Heap
    void processPickupQueue()
    {
        pickupQueue.displayContent();

        if (pickupQueue.isEmpty())
        {
            cout << "Pickup Queue is empty." << endl;
            return;
        }

        cout << "\n--- Moving Parcels to Sorting Engine ---" << endl;
        while (!pickupQueue.isEmpty())
        {
            Parcel* p = pickupQueue.dequeue();
            undo.push(p, p->getStatus(), "Moved to Sorter");
            p->setStatus("Sorting");
            sortingEngine.insert(p);
            cout << "Parcel " << p->getID() << " moved to Sorting Engine." << endl;
        }
    }

    // Option 3: Move from Sorter -> Warehouse (Sorted by Priority)
    void sortToWarehouse()
    {

        if (sortingEngine.isEmpty())
        {
            cout << "No parcels to sort." << endl;
            return;
        }
        cout << "\n--- Sorting based on Priority & Weight ---" << endl;
        while (!sortingEngine.isEmpty())
        {
            Parcel* p = sortingEngine.extractMin(); // Extract highest priority
            undo.push(p, p->getStatus(), "Moved to Warehouse");
            p->setStatus("In Warehouse Queue");
            warehouseQueue.enqueue(p);
            cout << "Parcel " << p->getID() << " sorted to Warehouse Queue." << endl;
        }
    }

    // Option 4: Assign Rider and calculate route
    void assignRider()
    {
        if (warehouseQueue.isEmpty())
        {
            cout << "Warehouse Queue is empty." << endl;
            return;
        }

        Parcel* p = warehouseQueue.dequeue();
        bool assigned = false;

        // Try to find a rider with enough capacity
        for (int i = 0; i < 3; i++)
        {
            if (riders[i].assignParcel(p))
            {
                p->setRiderId(riders[i].id); // Track which rider has the parcel

                undo.push(p, p->getStatus(), "Assigned Rider");
                p->setStatus("In Transit (Rider: " + riders[i].name + ")");
                p->addEvent("Picked up by " + riders[i].name);
                transitQueue.enqueue(p);

                cout << "Parcel " << p->getID() << " assigned to " << riders[i].name << endl;
                cout << "Calculating Route..." << endl;
                routingEngine.findShortestPath("Lahore", p->getDest());
                assigned = true;
                break;
            }
        }
        if (!assigned)
        {
            cout << "Alert: No rider has capacity for " << p->getWeight() << "kg parcel. Returned to Queue." << endl;
            warehouseQueue.enqueue(p); // Put back in queue if no rider found
        }
    }

    // Option 5a: Missing Parcel Logic
    void reportMissing(string id)
    {
        Parcel* p = trackingEngine.search(id);
        if (!p)
        {
            cout << "ID not found." << endl;
            return;
        }
        undo.push(p, p->getStatus(), "Reported Missing");
        p->setStatus("MISSING");
        p->markMissing(true);
        cout << "Parcel " << id << " flagged as MISSING." << endl;
    }

    // Option 5b: Undo Logic (Stack Pop)
    void undoLastOperation()
    {
        UndoState state;
        if (undo.pop(state))
        {
            cout << "UNDO: Reverting " << state.p->getID() << " from " << state.p->getStatus() << " to " << state.prevStatus << endl;
            state.p->setStatus(state.prevStatus);
        }
        else
        {
            cout << "Nothing to undo." << endl;
        }
    }

    void releaseRiderLoad(Parcel* p)
    {
        int rid = p->getRiderId();
        if (rid != -1)
        {
            int index = rid - 1;
            if (index >= 0 && index < 3)
            {
                riders[index].currentLoad -= p->getWeight();
                if (riders[index].currentLoad < 0) riders[index].currentLoad = 0;
                cout << " [System] Rider " << riders[index].name << " unloaded. Capacity Free: " << (riders[index].capacity - riders[index].currentLoad) << "kg" << endl;
            }
            p->setRiderId(-1);
        }
    }

    // Option 7: Simulation of delivery lifecycle
    void simulateParcelLifecycle(string id)
    {
        Parcel* p = trackingEngine.search(id);
        if (!p)
        {
            cout << "Parcel not found." << endl;
            return;
        }

        int choice;
        cout << "\nUpdate Status for " << id << ":" << endl;
        cout << "1. Unload at Destination" << endl;
        cout << "2. Attempt Delivery" << endl;
        cout << "3. Mark Delivered (Final)" << endl;
        cout << "4. Return to Sender" << endl;
        cout << "5. Exit" << endl;
        cout << "Choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            p->setStatus("Arrived at Dest Hub");
            p->addEvent("Unloaded at " + p->getDest() + " warehouse");
            releaseRiderLoad(p);
            cout << "Status updated." << endl;
            break;
        case 2:
            p->incrementAttempts();
            p->setStatus("Out for Delivery");
            p->addEvent("Delivery Attempt #" + to_string(p->getAttempts()));
            cout << "Status updated." << endl;
            break;
        case 3:
            p->setStatus("Delivered");
            p->addEvent("Final Delivery Successful");
            releaseRiderLoad(p);
            cout << "Status updated." << endl;
            break;
        case 4:
            p->setStatus("Returned");
            p->addEvent("Returned to Sender (Failed Delivery)");
            releaseRiderLoad(p);
            cout << "Status updated." << endl;
            break;
        }

    }

    // Option 6: Graph Edge Management
    void manageRoads()
    {
        string c1, c2;
        int op;
        cout << "1. Block Road\n2. Unblock Road\n3. Show Alternatives\n4. Exit\nChoice: ";
        cin >> op;

        if (op == 1)
        {
            cout << "Enter City 1: "; cin >> c1;
            cout << "Enter City 2: "; cin >> c2;
            routingEngine.blockRoad(c1, c2, true);
        }
        else if (op == 2)
        {
            cout << "Enter City 1: "; cin >> c1;
            cout << "Enter City 2: "; cin >> c2;
            routingEngine.blockRoad(c1, c2, false);
        }

        else if (op == 3)
        {
            cout << "Enter City 1: "; cin >> c1;
            cout << "Enter City 2: "; cin >> c2;
            routingEngine.findAllRoutes(c1, c2);
        }
        else return;
    }

    // Option 8: Parcel Tracking
    void track(string id)
    {
        Parcel* p = trackingEngine.search(id);
        if (p) p->printDetails();
        else cout << "Not Found." << endl;
    }
};

int main()
{
    CourierSystem cs;
    int choice;
    string id, dest;
    int prio;
    double weight;

    // Menu Loop
    while (true)
    {
        system("cls");
        cout << "============================================" << endl;
        cout << "            SWIFTEX COURIER ENGINE" << endl;
        cout << "============================================" << endl;
        cout << " 1. Register New Parcel" << endl;
        cout << " 2. Parcel Sorting (Pickup -> Sort)" << endl;
        cout << " 3. Move parcel to Warehouse Queue" << endl;
        cout << " 4. Assign Rider (Route Calculation)" << endl;
        cout << " 5. Report Missing Parcel & Undo last operation" << endl;
        cout << " 6. Manage Roads (Block/Unblock)" << endl;
        cout << " 7. Update Parcel Status" << endl;
        cout << " 8. Track Parcel" << endl;
        cout << " 9. Exit System" << endl;
        cout << "============================================" << endl;
        cout << " Select Option: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        cout << "\n";

        switch (choice)
        {
        case 1:
            cout << "--- [ Register New Parcel ] ---" << endl;
            cout << "Enter Parcel ID: "; cin >> id;
            cout << "Enter Priority (1=Overnight, 2=Two Day, 3=Normal): "; cin >> prio;
            cout << "Enter Weight (kg): "; cin >> weight;
            cout << "Enter Destination (Lahore/Islamabad/Karachi/Multan/Peshawar): "; cin >> dest;
            cs.registerParcel(id, prio, weight, dest);
            pause();
            break;

        case 2:
            cout << "--- [ Sorting Processing ] ---" << endl;
            cs.processPickupQueue();
            pause();
            break;

        case 3:
            cout << "--- [ Moving To Warehouse ] ---" << endl;
            cs.sortToWarehouse();
            pause();
            break;

        case 4:
            cout << "--- [ Assigning Rider ] ---" << endl;
            cs.assignRider();
            pause();
            break;

        case 5:
            cout << "1. Report Missing\n2. Undo Last Op\n3. Exit\nChoice: ";
            int sub; cin >> sub;
            if (sub == 1)
            {
                cout << "Enter ID: "; cin >> id; cs.reportMissing(id);
            }
            else if (sub == 2)
            {
                cs.undoLastOperation();
            }
            else
            {
                break;
            }
            pause();
            break;

        case 6:
            cout << "--- [ Road Management ] ---" << endl;
            cs.manageRoads();
            pause();
            break;

        case 7:
            cout << "--- [ Status Management ] ---" << endl;
            cout << "Enter Parcel ID to Update: "; cin >> id;
            cs.simulateParcelLifecycle(id);
            pause();
            break;

        case 8:
            cout << "--- [ Tracking System ] ---" << endl;
            cout << "Enter Parcel ID to Track: "; cin >> id;
            cs.track(id);
            pause();
            break;

        case 9:
            cout << "Shutting down system..." << endl;
            return 0;

        default:
            cout << "Invalid Option. Please try again." << endl;
            pause();
        }
    }
}