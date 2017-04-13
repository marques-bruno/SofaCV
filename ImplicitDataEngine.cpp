#include "ImplicitDataEngine.h"

namespace sofa
{
namespace OR
{
namespace common
{
void ImplicitDataEngine::checkData(bool call_callback)
{
	std::map<core::objectmodel::BaseData*, trackPair*> dataToUpdate;

	// Calling callbacks of all dirty data registered with "addDataCallback" and
	// cleaning them too
	for (std::map<core::objectmodel::BaseData*, trackPair*>::value_type& t :
			 m_trackers)
	{
		t.first->updateIfDirty();
		if (t.second->first->isDirty())
		{
			m_callback = t.second->second;
			if (call_callback) dataToUpdate.insert(t);
			t.first->cleanDirty();
			t.second->first->clean();
		}
	}
	for (auto t : dataToUpdate)
	{
		m_callback = t.second->second;
		(this->*m_callback)(t.first);
		t.first->cleanDirty();
		t.second->first->clean();
	}
}

bool ImplicitDataEngine::checkInputs()
{
  bool hasDirtyValues = false;

	checkData();

  // Calling callbacks of all dirty inputs registered with "addInput" returning
  // true if any of the inputs is dirty
	for (std::map<core::objectmodel::BaseData*, trackPair*>::value_type& t :
       m_inputs)
  {
    t.first->updateIfDirty();
		if (t.second->first->isDirty())
    {
			m_callback = t.second->second;
      (this->*m_callback)(t.first);
      hasDirtyValues = true;
    }
  }
  return hasDirtyValues;
}

void ImplicitDataEngine::clean()
{
  // Cleaning all inputs
	for (std::map<core::objectmodel::BaseData*, trackPair*>::value_type& t :
       m_inputs)
  {
		t.first->cleanDirty();
		t.second->first->clean();
  }
  // Setting modified output values to dirty.
  // TODO: Only set dirtyValue if the output was changed from within, so that
  // other engines get notified from the change
  for (std::map<core::objectmodel::BaseData*, core::DataTracker*>::value_type&
           p : m_outputs)
  {
    if (p.second->isDirty())
    {
      p.second->clean();
      p.first->setDirtyValue();
    }
  }
}

bool ImplicitDataEngine::_bindData(core::objectmodel::BaseData* data,
                                   const std::string& alias)
{
  const std::multimap<std::string, core::objectmodel::BaseData*>& dataMap =
      this->getDataAliases();

  for (auto& d : dataMap)
    if (d.first == alias)
    {
      data->setParent(d.second, "@" + this->getPathName() + "." + alias);
      return true;
    }
  ImplicitDataEngine* engine = getPreviousEngineInGraph();
  if (engine)
    return engine->_bindData(data, alias);
  else
    return false;
}

ImplicitDataEngine* ImplicitDataEngine::getPreviousEngineInGraph()
{
  std::vector<ImplicitDataEngine*> engines;
  getContext()->get<ImplicitDataEngine>(&engines);
  for (size_t i = 0; i < engines.size(); ++i)
    if (engines[i]->getName() == getName())
      return (i) ? (engines[i - 1]) : (NULL);
  return NULL;
}

void ImplicitDataEngine::_trackData(core::objectmodel::BaseData* data,
                                    DataCallback callback, TrackMap& map)
{
  core::DataTracker* tracker = new core::DataTracker();
  tracker->trackData(*data);
  data->setDirtyValue();
	map.insert(trackedData(data, new trackPair(tracker, callback)));
}

void ImplicitDataEngine::addDataCallback(core::objectmodel::BaseData* data,
                                         DataCallback callback)
{
  _trackData(data, callback, m_trackers);
}

void ImplicitDataEngine::addInput(core::objectmodel::BaseData* data,
                                  bool trackOnly, DataCallback callback)
{
	if (!d_autolink.getValue() || trackOnly || data->isSet())
  {
    _trackData(data, callback, m_inputs);
    return;
  }
  ImplicitDataEngine* engine = getPreviousEngineInGraph();
  if (engine)
  {
    bool isBinded = false;
    if (d_isLeft.getValue())
    {
      isBinded = engine->_bindData(data, data->getName() + "1_out");
      if (!isBinded)
        isBinded = engine->_bindData(data, data->getName() + "_out");
    }
    else
      isBinded = engine->_bindData(data, data->getName() + "2_out");

    if (!isBinded)
    {
      msg_warning(getName() + "::bindInputData()")
          << "couldn't bind input data " << data->getName()
          << " implicitly. Link is broken";
    }
		else
		{
      _trackData(data, callback, m_inputs);
			msg_advice(getName() + "::" + data->getName())
					<< "linked to " << data->getLinkPath()
					<< " implicitly. Please ensure that this makes sense. Otherwise, set "
						 "autolink to false";
		}
  }
  else
    msg_warning(getName() + "::bindInputData()")
        << "couldn't bind input data " << data->getName()
        << " implicitly. Link is broken";
}

void ImplicitDataEngine::addOutput(core::objectmodel::BaseData* data)
{
	if (m_inputs.find(data) != m_inputs.end()) return;
  core::DataTracker* tracker = new core::DataTracker();
  tracker->trackData(*data);
  data->cleanDirty();
  tracker->clean();
  m_outputs.insert(std::make_pair(data, tracker));
}

void ImplicitDataEngine::removeInput(core::objectmodel::BaseData* data)
{
	if (m_inputs.find(data) != m_inputs.end())
		m_inputs.erase(m_inputs.find(data));
}
void ImplicitDataEngine::removeOutput(core::objectmodel::BaseData* data)
{
	if (m_outputs.find(data) != m_outputs.end())
		m_outputs.erase(m_outputs.find(data));
}

void ImplicitDataEngine::removeDataCallback(core::objectmodel::BaseData* data)
{
	if (m_trackers.find(data) != m_trackers.end())
		m_trackers.erase(m_trackers.find(data));
}

}  // namespace common
}  // namespace OR
}  // namespace sofa
