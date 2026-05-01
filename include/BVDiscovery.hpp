#pragma once
#include <string>
#include <chrono>
#include <queue>
#include <list>
#include <boost/asio.hpp>
#include <condition_variable>
#include <mutex>
#include "linked_list.h"
#include "const.h"
#include "BVService.hpp"

class BVDiscovery
{
private:
    BVStatus status = BVStatus::BVSTATUS_IN_PROGRESS;
    std::atomic<bool> isBrowsingActive = false; // rpbobably will have to be atomic, since second mailbox thread will change this
    LinkedList_str* c_ll_p = NULL; // C linked list, for processing daemon responses
    // TODO: LinkedList_str should be wrapped in a unique ptr.

    const BVServiceData hostData; // for service data

    virtual void CreateConnectionContext(void) = 0; // private member function which actually starts Discovery
    // This shouldn't be virtual, as there are problems to cast from unique/shared ptr pting to std::any
    // To the desired implementation-oriented type.
    // virtual std::shared_ptr<std::any> CreateResolveContext(const BVServiceBrowseInstance& bI) = 0; // create resolver
    virtual void DestroyResolveContext(std::unique_ptr<std::any> rcp) = 0;
    virtual void Setup(void) = 0;
    virtual void Browse() = 0;

    std::thread worker_thread;

protected:
    std::list<BVServiceBrowseInstance> ReturnListFromBrowseResults(void)
    {
        std::list<BVServiceBrowseInstance> l;
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
            regType.erase(std::remove(regType.begin(), regType.end(), '\0'), regType.end());
            replyDomain.erase(std::remove(replyDomain.begin(), replyDomain.end(), '\0'), replyDomain.end());
            serviceName.erase(std::remove(serviceName.begin(), serviceName.end(), '\0'), serviceName.end());

            bI.regType = regType;
            bI.replyDomain = replyDomain;
            bI.serviceName = serviceName;
            l.push_back(bI);
            lle_p = lle_p->next_p;
        }
        return l;
    }

    // Resolve a service name discovered to a target host name, port number, and txt record.
    /*  
     * Please note, that resolve context and resolve operation is PER service to be resolved!
     * This means that it is a dynamic entity and must be freed after resolution is complete.
    */
    virtual BVStatus ResolveService(const BVServiceBrowseInstance& bI) = 0;
    BVStatus OnResolveRequest(std::unique_ptr<std::any> dp)
    {
        if (dp == nullptr)
        {
            std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESOLVE callback. " <<
                            "No data has been passed into the callback!" << std::endl;
            return BVStatus::BVSTATUS_FATAL_ERROR;
        }
        BVServiceBrowseInstance bI;
        try
        {
            bI = std::any_cast<BVServiceBrowseInstance>(*dp);
        }
        catch(const std::bad_any_cast& e)
        {
            std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESOLVE callback. " 
                        << e.what() << std::endl;
            return BVStatus::BVSTATUS_FATAL_ERROR;
        }
        return ResolveService(bI);
    }

    uint8_t GetDidServiceRegister(void)
    {
        return this->GetLinkedList_p()->didRegister;
    }

public:
    BVDiscovery(const BVServiceData _hostData) :
    hostData(_hostData)
    {
        this->c_ll_p = LinkedList_str_Constructor(NULL);
    }

    BVDiscovery(BVDiscovery&&) = default;
    BVDiscovery& operator=(BVDiscovery&&) = default;

    BVDiscovery(BVDiscovery &) = delete;
    BVDiscovery& operator=(BVDiscovery &) = delete;

    virtual ~BVDiscovery()
    {
        LinkedList_str_Destructor(&this->GetLinkedList_p());
    };

    void LaunchWorkingThread(void)
    {
        this->worker_thread = std::thread([this] {
            this->Browse();
        });
    }

    void TryJoinWorkerThread(void)
    {
        if (this->worker_thread.joinable())
        {
            this->worker_thread.join();
        }
    }

    std::thread& GetWorkerThread(void)
    {
        return this->worker_thread;
    }

    bool GetIsBrowsingActive(void)
    {
        return this->isBrowsingActive;
    }

    void SetIsBrowsingActive(const bool f)
    {
        this->isBrowsingActive = f;
    }

    BVServiceData GetHostData(void) const
    {
        return this->hostData;
    }

    // without '&' (reference),
    // the function would return an automatically allocated
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

    BVStatus& GetStatus(void)
    {
        return this->status;
    }
};