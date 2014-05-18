#include <cstdio>

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <iostream>
#include <windows.h>

#include <string>

using namespace BWAPI;

void drawStats();
void drawBullets();
void drawVisibilityData();
void showPlayers();
void showForces();
bool show_bullets;
bool show_visibility_data;

Unitset units    = NULL;
Position nullPosition = NULL;
PositionOrUnit flag1 = nullPosition;
PositionOrUnit flag2 = nullPosition;
void patrolFlag(Unitset units, PositionOrUnit flag1, PositionOrUnit flag2, bool shiftQueueCommand);

void reconnect()
{
  while(!BWAPIClient.connect())
  {
    Sleep(1000);
  }
}

int main(int argc, const char* argv[])
{
  std::cout << "Connecting..." << std::endl;;
  reconnect();
  while(true)
  {
    std::cout << "waiting to enter match" << std::endl;
    while ( !Broodwar->isInGame() )
    {
      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected())
      {
        std::cout << "Reconnecting..." << std::endl;;
        reconnect();
      }
    }
    std::cout << "starting match!" << std::endl;
    Broodwar->sendText("AI said: Hello world!");
    Broodwar << "The map is " << Broodwar->mapName() << ", a " << Broodwar->getStartLocations().size() << " player map" << std::endl;
    // Enable some cheat flags
    Broodwar->enableFlag(Flag::UserInput);
    // Uncomment to enable complete map information
    //Broodwar->enableFlag(Flag::CompleteMapInformation);
   
    show_bullets=false;
    show_visibility_data=false;

    if (Broodwar->isReplay())
    {
      Broodwar << "The following players are in this replay:" << std::endl;;
      Playerset players = Broodwar->getPlayers();
      for(Playerset::iterator p = players.begin(); p != players.end(); ++p )
      {
        if ( !p->getUnits().empty() && !p->isNeutral() )
          Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
      }
    }
    else
    {
      Broodwar << "The match up is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
	  Unitset allUnits    = Broodwar->self()->getUnits();
      for ( Unitset::iterator i = allUnits.begin(); i != allUnits.end(); ++i )
      {
		  if ( i->getType().isPowerup() )
		  {
			  if (!flag1.isUnit())
			  {
				  flag1 = (*i);
				  //Broodwar << "Flag 1 X - " << flag1.getPosition().x << " F1 Y - " << flag1.getPosition().y << std::endl;
			  }
			  else if (!flag2.isUnit())
			  {
				  flag2 = (*i);
				  //Broodwar << "Flag 2 X - " << flag2.getPosition().x << " F2 Y - " << flag2.getPosition().y << std::endl;
			  }
		  }
		  else if ( i->getType() == BWAPI::UnitTypes::Terran_Marine )
		  {
			  units.insert((*i));
		  }
      }
    }
    while(Broodwar->isInGame())
    {
      for(std::list<Event>::const_iterator e = Broodwar->getEvents().begin(); e != Broodwar->getEvents().end(); ++e)
      {
        switch(e->getType())
        {
          case EventType::MatchEnd:
            if (e->isWinner())
              Broodwar << "I won the game" << std::endl;
            else
              Broodwar << "I lost the game" << std::endl;
            break;
          case EventType::SendText:
            if (e->getText()=="/show bullets")
            {
              show_bullets=!show_bullets;
            } else if (e->getText()=="/show players")
            {
              showPlayers();
            } else if (e->getText()=="/show forces")
            {
              showForces();
            } else if (e->getText()=="/show visibility")
            {
              show_visibility_data=!show_visibility_data;
            } 
            else
            {
              Broodwar << "You typed \"" << e->getText() << "\"!" << std::endl;
            }
            break;
          case EventType::ReceiveText:
            Broodwar << e->getPlayer()->getName() << " said \"" << e->getText() << "\"" << std::endl;
            break;
          case EventType::PlayerLeft:
            Broodwar << e->getPlayer()->getName() << " left the game." << std::endl;
            break;
          case EventType::NukeDetect:
            if (e->getPosition()!=Positions::Unknown)
            {
              Broodwar->drawCircleMap(e->getPosition(), 40, Colors::Red, true);
              Broodwar << "Nuclear Launch Detected at " << e->getPosition() << std::endl;
            }
            else
              Broodwar << "Nuclear Launch Detected" << std::endl;
            break;
          case EventType::UnitCreate:
            if (!Broodwar->isReplay())
			{
              //Broodwar << "A " << e->getUnit()->getType() << " [" << e->getUnit() << "] has been created at " << e->getUnit()->getPosition() << std::endl;
			}
			else
            {
              // if we are in a replay, then we will print out the build order
              // (just of the buildings, not the units).
              if (e->getUnit()->getType().isBuilding() && e->getUnit()->getPlayer()->isNeutral()==false)
              {
                int seconds=Broodwar->getFrameCount()/24;
                int minutes=seconds/60;
                seconds%=60;
                Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, e->getUnit()->getPlayer()->getName().c_str(), e->getUnit()->getType().c_str());
              }
            }
            break;
          case EventType::UnitDestroy:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%x] has been destroyed at (%d,%d)",e->getUnit()->getType().c_str(), e->getUnit(), e->getUnit()->getPosition().x, e->getUnit()->getPosition().y);
            break;
          case EventType::UnitMorph:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",e->getUnit()->getType().c_str(), e->getUnit(), e->getUnit()->getPosition().x, e->getUnit()->getPosition().y);
            else
            {
              // if we are in a replay, then we will print out the build order
              // (just of the buildings, not the units).
              if (e->getUnit()->getType().isBuilding() && e->getUnit()->getPlayer()->isNeutral()==false)
              {
                int seconds=Broodwar->getFrameCount()/24;
                int minutes=seconds/60;
                seconds%=60;
                Broodwar->sendText("%.2d:%.2d: %s morphs a %s" ,minutes, seconds, e->getUnit()->getPlayer()->getName().c_str(), e->getUnit()->getType().c_str());
              }
            }
            break;
          case EventType::UnitShow:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)", e->getUnit()->getType().c_str(), e->getUnit(), e->getUnit()->getPosition().x, e->getUnit()->getPosition().y);
            break;
          case EventType::UnitHide:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%x] was last seen at (%d,%d)", e->getUnit()->getType().c_str(), e->getUnit(), e->getUnit()->getPosition().x, e->getUnit()->getPosition().y);
            break;
          case EventType::UnitRenegade:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%x] is now owned by %s", e->getUnit()->getType().c_str(), e->getUnit(), e->getUnit()->getPlayer()->getName().c_str());
            break;
          case EventType::SaveGame:
            Broodwar->sendText("The game was saved to \"%s\".", e->getText().c_str());
            break;
        }
      }

      if (show_bullets)
        drawBullets();

      if (show_visibility_data)
        drawVisibilityData();

      //drawStats();
      //Broodwar->drawTextScreen(300,0,"FPS: %f",Broodwar->getAverageFPS());

	  patrolFlag(units, flag1, flag2, false);

      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected())
      {
        std::cout << "Reconnecting..." << std::endl;
        reconnect();
      }
    }
    std::cout << "Game ended" << std::endl;
  }
  std::cout << "Press ENTER to continue..." << std::endl;
  std::cin.ignore();
  return 0;
}

void drawStats()
{
  int line = 0;
  for ( UnitType::set::iterator i = UnitTypes::allUnitTypes().begin(); i != UnitTypes::allUnitTypes().end(); ++i )
  {
	  /*
		Interest functions in unit.h
		isPatrolling()
		isStuck()
		attack(PositionOrUnit target, bool shiftQueueCommand = false);
		patrol()
		move()
		functions "can"
		canMoveGrouped
		canPatrolGrouped

		isPowerup ()
		isFlagBeacon ()
	  const UnitType BWAPI::UnitTypes::Terran_Marine
	  */
    int count = Broodwar->self()->allUnitCount(*i);
    if ( count )
    {
		if ( (*i).isPowerup() )
		{
			Broodwar->drawTextScreen(5, 26*line, "P - %d %s%c", count, (*i).c_str(), count == 1 ? ' ' : 's');
			++line;
		}
		if ( (*i) == BWAPI::UnitTypes::Terran_Marine )
		{
			Broodwar->drawTextScreen(5, 26*line, "T - %d %s%c", count, (*i).c_str(), count == 1 ? ' ' : 's');
			++line;
		}
    }
  }
}

void drawBullets()
{
  Bulletset bullets = Broodwar->getBullets();
  for(Bulletset::iterator i = bullets.begin(); i != bullets.end(); ++i)
  {
    Position p = i->getPosition();
    double velocityX = i->getVelocityX();
    double velocityY = i->getVelocityY();
    Broodwar->drawLineMap(p, p + Position((int)velocityX, (int)velocityY), i->getPlayer() == Broodwar->self() ? Colors::Green : Colors::Red);
    Broodwar->drawTextMap(p, "%c%s", i->getPlayer() == Broodwar->self() ? Text::Green : Text::Red, i->getType().c_str());
  }
}

void drawVisibilityData()
{
  int wid = Broodwar->mapHeight(), hgt = Broodwar->mapWidth();
  for ( int x = 0; x < wid; ++x )
    for ( int y = 0; y < hgt; ++y )
    {
      if ( Broodwar->isExplored(x, y) )
        Broodwar->drawDotMap(x*32+16, y*32+16, Broodwar->isVisible(x, y) ? Colors::Green : Colors::Blue);
      else
        Broodwar->drawDotMap(x*32+16, y*32+16, Colors::Red);
    }
}

void showPlayers()
{
  Playerset players = Broodwar->getPlayers();
  for(Playerset::iterator i = players.begin(); i != players.end(); ++i)
    Broodwar << "Player [" << i->getID() << "]: " << i->getName() << " is in force: " << i->getForce()->getName() << std::endl;
}

void showForces()
{
  Forceset forces=Broodwar->getForces();
  for(Forceset::iterator i = forces.begin(); i != forces.end(); ++i)
  {
    Playerset players = i->getPlayers();
    Broodwar << "Force " << i->getName() << " has the following players:" << std::endl;
    for(Playerset::iterator j = players.begin(); j != players.end(); ++j)
      Broodwar << "  - Player [" << j->getID() << "]: " << j->getName() << std::endl;
  }
}

void patrolFlag(Unitset units, PositionOrUnit flag1, PositionOrUnit flag2, bool shiftQueueCommand )
{
	Unit closestFlag = NULL;
	if ( flag1.isUnit() && flag2.isUnit() )
	{
		for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
		{
			if ( i->getType() == BWAPI::UnitTypes::Terran_Marine )
			{
				if ( i->getDistance(flag1) < 30 )
				{
					i->rightClick(flag2, shiftQueueCommand);
				}
				else if ( i->getDistance(flag2) < 30 )
				{
					i->rightClick(flag1, shiftQueueCommand);
				}
				else if ( i->isIdle() )
				{
					if ( i->getDistance(flag1) < i->getDistance(flag2) )
					{
						i->rightClick(flag1, shiftQueueCommand);
					}
					else
					{
						i->rightClick(flag2, shiftQueueCommand);
					}
				}
			}
		}
	}
}