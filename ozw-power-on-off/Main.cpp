// Main.cpp
// Author: Steven D. Luland
// The main class for a prototype z-wave system.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "Options.h"
#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Group.h"
#include "Notification.h"
#include "ValueStore.h"
#include "Value.h"
#include "ValueBool.h"
#include "Log.h"

using namespace OpenZWave;

static uint32 g_homeId;
static bool   g_initFailed = false;

typedef struct
{
	uint32		m_homeId;
	uint8		m_nodeId;
	bool		m_polled;
	list<ValueID>	m_values;
}NodeInfo;

static list<NodeInfo*> g_nodes;
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;
//-----------------------------------------------------------------------------
// <GetNodeInfo>
// Return the NodeInfo object associated with this notification
//-----------------------------------------------------------------------------
NodeInfo* GetNodeInfo
(
	Notification const* _notification
)
{
	uint32 const homeId = _notification->GetHomeId();
	uint8 const nodeId = _notification->GetNodeId();
	for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
	{
		NodeInfo* nodeInfo = *it;
		if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
		{
			return nodeInfo;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// <OnNotification>
// Callback that is triggered when a value, group or node changes
//-----------------------------------------------------------------------------
void OnNotification
(
	Notification const* _notification,
	void* _context
)
{
	// Must do this inside a critical section to avoid conflicts with the main thread
	pthread_mutex_lock( &g_criticalSection );

	switch( _notification->GetType() )
	{
		case Notification::Type_ValueAdded:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				// Add the new value to our list
				nodeInfo->m_values.push_back( _notification->GetValueID() );
			}
			break;
		}

		case Notification::Type_ValueRemoved:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				// Remove the value from out list
				for( list<ValueID>::iterator it = nodeInfo->m_values.begin(); it != nodeInfo->m_values.end(); ++it )
				{
					if( (*it) == _notification->GetValueID() )
					{
						nodeInfo->m_values.erase( it );
						break;
					}
				}
			}
			break;
		}

		case Notification::Type_ValueChanged:
		{
			// One of the node values has changed
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo = nodeInfo;		// placeholder for real action
			}
			break;
		}

		case Notification::Type_Group:
		{
			// One of the node's association groups has changed
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo = nodeInfo;		// placeholder for real action
			}
			break;
		}

		case Notification::Type_NodeAdded:
		{
			// Add the new node to our list
			NodeInfo* nodeInfo = new NodeInfo();
			nodeInfo->m_homeId = _notification->GetHomeId();
			nodeInfo->m_nodeId = _notification->GetNodeId();
			nodeInfo->m_polled = false;
			g_nodes.push_back( nodeInfo );
			break;
		}

		case Notification::Type_NodeRemoved:
		{
			// Remove the node from our list
			uint32 const homeId = _notification->GetHomeId();
			uint8 const nodeId = _notification->GetNodeId();
			for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
			{
				NodeInfo* nodeInfo = *it;
				if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
				{
					g_nodes.erase( it );
					delete nodeInfo;
					break;
				}
			}
			break;
		}

		case Notification::Type_NodeEvent:
		{
			// We have received an event from the node, caused by a
			// basic_set or hail message.
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo = nodeInfo;		// placeholder for real action
			}
			break;
		}

		case Notification::Type_PollingDisabled:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo->m_polled = false;
			}
			break;
		}

		case Notification::Type_PollingEnabled:
		{
			if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
			{
				nodeInfo->m_polled = true;
			}
			break;
		}

		case Notification::Type_DriverReady:
		{
			g_homeId = _notification->GetHomeId();
			break;
		}

		case Notification::Type_DriverFailed:
		{
			g_initFailed = true;
			pthread_cond_broadcast(&initCond);
			break;
		}

		case Notification::Type_AwakeNodesQueried:
		case Notification::Type_AllNodesQueried:
		case Notification::Type_AllNodesQueriedSomeDead:
		{
			pthread_cond_broadcast(&initCond);
			break;
		}

		case Notification::Type_DriverReset:
		case Notification::Type_Notification:
		case Notification::Type_NodeNaming:
		case Notification::Type_NodeProtocolInfo:
		case Notification::Type_NodeQueriesComplete:
		default:
		{
		}
	}

	pthread_mutex_unlock( &g_criticalSection );
}

void SetValue(bool value)
{
    int nodeid = 3;
    pthread_mutex_lock( &g_criticalSection );
    for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
    {
	NodeInfo* nodeInfo = *it;
	if( nodeInfo->m_nodeId != nodeid ) continue;
	for( list<ValueID>::iterator it2 = nodeInfo->m_values.begin();
 it2 != nodeInfo->m_values.end(); ++it2 )
	{
	    ValueID v = *it2;
	    if( v.GetCommandClassId() == 0x25)
	    {
		bool* status;
		printf("\n Setting Node %d to %s ",
		       nodeInfo->m_nodeId,
		       value ? "On" : "Off");
		Manager::Get()->SetValue(v, value);
		printf("\n Node %d is now %s \n",
		       nodeInfo->m_nodeId,
		       Manager::Get()->GetValueAsBool(v, status) ? "ON" : "OFF");

		break;
	    }
	}
    }

    pthread_mutex_unlock( &g_criticalSection );
}

int main(int argc, char* argv[])
{
    string port = "/dev/ttyUSB0";
    pthread_mutexattr_t mutexattr;

   // Set up mutual exclusion so that this thread has priority
    pthread_mutexattr_init ( &mutexattr );
    pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &g_criticalSection, &mutexattr );
    pthread_mutexattr_destroy( &mutexattr );

    pthread_mutex_lock( &initMutex );

    printf("\n Creating Options \n");

    Options::Create( "/usr/local/etc/openzwave", "./meta/", "" );
    Options::Get()->AddOptionInt( "SaveLogLevel", LogLevel_Detail );
    Options::Get()->AddOptionInt( "QueueLogLevel", LogLevel_Debug );
    Options::Get()->AddOptionInt( "DumpTrigger", LogLevel_Error );
    Options::Get()->AddOptionInt( "PollInterval", 500 );

    // Comment the following line out if you want console logging
    Options::Get()->AddOptionBool( "ConsoleOutput", false );

    Options::Get()->AddOptionBool( "IntervalBetweenPolls", true );
    Options::Get()->AddOptionBool("ValidateValueChanges", true);
    Options::Get()->Lock();

    printf("\n Creating Manager \n");

    Manager::Create();
    Manager::Get()->AddWatcher( OnNotification, NULL );
    Manager::Get()->AddDriver( port );

    // Release the critical section
    pthread_cond_wait( &initCond, &initMutex );

    printf("\n Starting On/Off Program \n");

    // loop to switch on/off every 5 seconds
    for (int i = 0; i < 5; i++ )
    {
      printf("\n Loop %i \n ====== \n", i+1 );
      SetValue(true);
      sleep(5);
      // Why is this not working anymore?
      SetValue(false);
      sleep(5);
    }

    // program exit (clean up)
    if( strcasecmp( port.c_str(), "usb" ) == 0 )
      {
	Manager::Get()->RemoveDriver( "HID Controller" );
      }
    else
      {
	Manager::Get()->RemoveDriver( port );
      }
    Manager::Get()->RemoveWatcher( OnNotification, NULL );
    Manager::Destroy();
    Options::Destroy();
    pthread_mutex_destroy( &g_criticalSection );

    return 0;
}
