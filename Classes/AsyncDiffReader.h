
#pragma once

#include <cstdio>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "json_safe.hpp"
#include "UICommon.h"
#include "UIConfig.h"
#include "Common.h"
#include "Utilities.h"

namespace jevo
{
  
  class DiffItem
  {
  public:
    PixelPos sourseX = 0;
    PixelPos sourseY = 0;
    PixelPos destX = 0;
    PixelPos destY = 0;
    cocos2d::Color3B color;
    std::string action;
    uint64_t id = -1;
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
        item.action = d["a"];
        item.id = d["id"];
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
          
          m_inProccess = true;
          m_performUpdate = false;
        }
        
        m_lastUpdateDuration = 0.0;
        
        std::string filePath = m_wordkingFolder;
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(4) << m_fileIndex;
        filePath += "/" + stream.str() + ".json";
        
        if (m_updates.ReadFromFile(filePath))
        {
          m_fileIndex += 1;
          if (jevo::config::removeFiles)
          {
            std::remove(filePath.c_str());
          }
        }

        {
          std::lock_guard<std::mutex> lk(m_lock);
          m_inProccess = false;
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

