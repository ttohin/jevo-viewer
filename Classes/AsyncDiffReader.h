
#pragma once

#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "json.hpp"
#include "UICommon.h"

namespace jevo
{
  class DiffItem
  {
  public:
    uint sourseX = 0;
    uint sourseY = 0;
    uint destX = 0;
    uint destY = 0;
    cocos2d::Color3B color;
  };
  
  using DiffItemVector = std::vector<DiffItem>;
  
  class DiffSequence
  {
  public:
    
    bool ReadFromFile(const std::string& fileName)
    {
      std::ifstream i(fileName);
      if (!i)
        return false;
      
      nlohmann::json json;
      i >> json;
      
      if (json.is_null())
        return false;
      
      m_seq.clear();
      m_seq.reserve(json.size());
      
      for (const auto& d : json)
      {
        DiffItem item;
        item.sourseX = d["sx"];
        item.sourseY = d["sy"];
        item.destX = d["dx"];
        item.destY = d["dy"];
        item.color = graphic::ColorFromUint(d["c"]);
        m_seq.push_back(item);
      }
      
      return true;
    }
    
    DiffItemVector m_seq;
  };
  
  class AsyncDiffReader
  {
  public:
    AsyncDiffReader()
    {
    }
    
    bool Init(const std::string& workingFolder)
    {
      m_wordkingFolder = workingFolder;
      m_thread = std::thread(&AsyncDiffReader::WorkerThread, this);
      return true;
    }
    
    ~AsyncDiffReader()
    {
      m_thread.join();
    }
    
    void WorkerThread()
    {
      while (1)
      {
        unsigned int fileIndex = 0;
        
        {
          std::unique_lock<std::mutex> lk(m_lock);
          m_semaphore.wait(lk, [this]
                           {
                             return m_performUpdate || m_shouldStop;
                           });
          
          if (m_shouldStop)
          {
            return;
          }
          
          fileIndex = m_fileIndex;
          m_fileIndex += 1;
          m_inProccess = true;
          m_performUpdate = false;
        }
        
        struct timeval tv;
        struct timeval start_tv;
        gettimeofday(&start_tv, NULL);
        m_lastUpdateDuration = 0.0;
        
        
        std::string filePath = m_wordkingFolder;
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(4) << fileIndex;
        filePath += stream.str() + ".json";
        
        m_updates.ReadFromFile(filePath);
        
        gettimeofday(&tv, NULL);
        double lastUpdateDuration = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000000.0;
        
        {
          std::lock_guard<std::mutex> lk(m_lock);
          m_inProccess = false;
          m_lastUpdateDuration = lastUpdateDuration;
        }
      }
    }
    
    bool IsAvailable()
    {
      std::lock_guard<std::mutex> lk(m_lock);
      return !m_inProccess;
    }
    
    double GetLastUpdateTime()
    {
      return m_lastUpdateDuration;
    }
    
    void LoadNext()
    {
      std::lock_guard<std::mutex> lk(m_lock);
      m_performUpdate = true;
      m_semaphore.notify_one();
    }
    
    void Stop()
    {
      std::lock_guard<std::mutex> lk(m_lock);
      m_shouldStop = true;
      m_semaphore.notify_one();
    }
    
    void PopDiffs(DiffItemVector& output)
    {
      assert(IsAvailable());
      m_updates.m_seq.swap(output);
    }
    
  private:
    bool m_performUpdate = false;
    bool m_shouldStop = false;
    bool m_inProccess = false;
    double m_lastUpdateDuration;
    unsigned int m_fileIndex = 0;
    std::string m_wordkingFolder;
    std::mutex m_lock;
    std::condition_variable m_semaphore;
    std::thread m_thread;
    DiffSequence m_updates;
  };
}

