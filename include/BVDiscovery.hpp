#pragma once
#include <string>
#include <chrono>
#include <boost/asio.hpp>
#include "linked_list.h"
#include "BVService.hpp"

#define DISCOVERY_TIMER_TRIGGER_S 5
#define N_BYTES_SERVNAME_MAX      24
#define N_BYTES_REGTYPE_MAX       24
#define N_BYTES_REPLDOMN_MAX      16
#define N_BYTES_SERVICE_STR_TOTAL (N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX + N_BYTES_REPLDOMN_MAX)
#define N_SERVICES_MAX            32

class BVDiscovery
{
private:
    BVStatus status = BVStatus::BVSTATUS_IN_PROGRESS;
    bool isBrowsingActive = false;
    LinkedList_str* c_ll_p = NULL; // C linked list, for processing daemon responses
    // TODO: LinkedList_str should be wrapped in a unique ptr.

    const BVServiceHostData hostData; // for service data

    std::mutex& discoveryQueueMutex;
    std::condition_variable& discoveryQueueCV;
    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue_p;
    bool& isDiscoveryQueueReady;

    virtual void CreateConnectionContext(void) = 0; // private member function which actually starts
    // void DestroyConnectionContext
    virtual void Setup(void) = 0;
    virtual void run() = 0;

protected:
    void PushBrowsedServicesToQueue(void)
    {
        for (const LinkedListElement_str* lle_p = this->c_ll_p->head_p;
            lle_p != NULL;)
        {
            BVServiceBrowseInstance bI; // put on heap? No, STL containers have elements allocated on heap.
            std::string regType(lle_p->data + N_BYTES_SERVNAME_MAX, N_BYTES_REGTYPE_MAX);
            std::string replyDomain(lle_p->data + N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX, N_BYTES_REPLDOMN_MAX);
            std::string serviceName(lle_p->data, N_BYTES_SERVNAME_MAX);

            regType.erase(std::remove(regType.begin(), regType.end(), ' '), regType.end());
            replyDomain.erase(std::remove(replyDomain.begin(), replyDomain.end(), ' '), replyDomain.end());
            serviceName.erase(std::remove(serviceName.begin(), serviceName.end(), ' '), serviceName.end());

            bI.regType = regType;
            bI.replyDomain = replyDomain;
            bI.serviceName = serviceName;
            this->discoveryQueue_p->push(bI);
            lle_p = lle_p->next_p;
        }
    }

public:
    BVDiscovery(const BVServiceHostData _hostData,
                std::mutex& _discoveryQueueMutex,
                std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                std::condition_variable& _discoveryQueueCV,
                bool& _isDiscoveryQueueReady) :
    hostData(_hostData),
    discoveryQueueMutex(_discoveryQueueMutex),
    discoveryQueue_p(_discoveryQueue),
    discoveryQueueCV(_discoveryQueueCV),
    isDiscoveryQueueReady(_isDiscoveryQueueReady) 
    {
        this->c_ll_p = LinkedList_str_Constructor(NULL);
    }

    virtual ~BVDiscovery()
    {
        LinkedList_str_Destructor(&this->GetLinkedList_p());
    };

    // Public interface TODO: Provide base implementation
    virtual void Shutdown(){};
    virtual void Start(){};

    virtual void OnShutdown(){};
    virtual void OnStart(){};

    // Function that is called upon running the discovery functionality when being passed
    // as callable to a thread
    void operator()(void)
    {
        this->run();
    }

    std::mutex& GetDiscoveryQueueMutex(void)
    {
        return this->discoveryQueueMutex;
    }

    bool& GetIsDiscoveryQueueReady(void)
    {
        return this->isDiscoveryQueueReady;
    }

    void SetIsDiscoveryQueueReady(const bool& f)
    {
        this->isDiscoveryQueueReady = f;
    }

    bool& GetIsBrowsingActive(void)
    {
        return this->isBrowsingActive;
    }

    std::condition_variable& GetDiscoveryQueueCV(void)
    {
        return this->discoveryQueueCV;
    }

    void SetIsBrowsingActive(const bool& f)
    {
        this->isBrowsingActive = f;
    }

    BVServiceHostData GetHostData(void) const
    {
        return this->hostData;
    }

    // without '&' (reference),
    // the function would return a automatically allocated
    // copy of the pointer, of which the address would get deallocated when
    // returning from this function
    LinkedList_str*& GetLinkedList_p(void)
    {
        return this->c_ll_p;
    }

    void SetStatus(const BVStatus& s)
    {
        this->status = s;
    }
};