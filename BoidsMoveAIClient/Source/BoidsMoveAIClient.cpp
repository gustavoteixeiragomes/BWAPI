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
Position flagPosition = 0;
void boidsPatrolFlag(Unitset units, PositionOrUnit flag1, PositionOrUnit flag2);

Position rule1(Unitset units, int unitID);
Position rule2(Unitset units, int unitID);
Position rule3(Unitset units, int unitID);
Position rule4(Position unitPosition, Position place);

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

	  boidsPatrolFlag(units, flag1, flag2);

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
    int count = Broodwar->self()->allUnitCount(*i);
    if ( count )
    {
		if ( (*i).isPowerup() )
		{
			Broodwar->drawTextScreen(5, 26*line, "P - %d %s%c", count, (*i).c_str(), count == 1 ? ' ' : 's');
			++line;
		}
		else if ( (*i) == BWAPI::UnitTypes::Terran_Marine )
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

void boidsPatrolFlag(Unitset units, PositionOrUnit flag1, PositionOrUnit flag2 )
{
	Position v1, v2, v3, v4, newPosition, unitVelocity;
	Position distance1 = units.getPosition() - flag1.getPosition();
	Position distance2 = units.getPosition() - flag2.getPosition();
	//Position flagPosition = 0;

	int line = 0;
	/*
	Broodwar->drawTextScreen(5, 10*line, "P - %d : %d", units.getPosition().x, units.getPosition().y);
	line++;
	Broodwar->drawTextScreen(5, 10*line, "P - %d : %d", distance1.x, distance1.y);
	line++;
	Broodwar->drawTextScreen(5, 10*line, "P - %d : %d", distance2.x, distance2.y);
	line++;
	*/
	if ( units.getPosition().getDistance(flag1.getPosition()) < 50 )
	{
		flagPosition = flag2.getPosition();
		Broodwar->drawTextScreen(5, 10*line, "Flag 2");
		line++;
	}
	else if ( units.getPosition().getDistance(flag2.getPosition()) < 50 )
	{
		flagPosition = flag1.getPosition();
		Broodwar->drawTextScreen(5, 10*line, "Flag 1");
		line++;
	}
	else if ( flagPosition.x == 0 && flagPosition.y == 0 )
	{
		if ( units.getPosition().getDistance(flag1.getPosition()) <  units.getPosition().getDistance(flag2.getPosition()) )
		{
			flagPosition = flag1.getPosition();
			Broodwar->drawTextScreen(5, 10*line, "Flag 1 B");
			line++;
		}
		else
		{
			flagPosition = flag2.getPosition();
			Broodwar->drawTextScreen(5, 10*line, "Flag 2 B");
			line++;
		}
	}

	for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
	{
		v1 = rule1( units, i->getID() );
		v2 = rule2( units, i->getID() );
		v3 = rule3( units, i->getID() );
		
		Broodwar->drawTextScreen(5, 10*line, "Flag 1: %d | Flag2: %d | P: %d", i->getDistance(flag1), i->getDistance(flag2), flagPosition.x);
		line++;
		
		v4 = rule4( i->getPosition(), flagPosition );
		//Broodwar->drawTextScreen(5, 10*line, "FP - %d : %d", v4.x, v4.y);
		//line++;
		//flagPosition = flag1.getPosition();
		//v4 = rule4( i->getPosition(), flagPosition );
		
		/*
		Broodwar->drawTextScreen(5, 10*line, "v1 - %d : %d", v1.x, v1.y);
		line++;
		Broodwar->drawTextScreen(5, 10*line, "v2 - %d : %d", v2.x, v2.y);
		line++;
		Broodwar->drawTextScreen(5, 10*line, "v3 - %d : %d", v3.x, v3.y);
		line++;
		*/
		Broodwar->drawTextScreen(5, 10*line, "v4 - %d : %d", v4.x, v4.y);
		line++;
		/*
		line -= 4;
		*/

		unitVelocity.x = (int)ceil(i->getVelocityX());
		unitVelocity.y = (int)ceil(i->getVelocityY());	
		unitVelocity = unitVelocity + ( v1*2 ) + ( v2*1 ) + (v3*2 ) + ( v4*4 );
		newPosition = i->getPosition() + unitVelocity;
		
		i->move(newPosition);
	}
}

// Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
Position rule1(Unitset units, int unitID)
{
	Position v1 = 0;
	Position unitPosition = 0;
	int total = units.size() - 1;
	for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( unitID == i->getID() )
		{
			unitPosition = i->getPosition();
		}
		else
		{
			v1 += ( i->getPosition() / total );
		}
	}
	v1 = ( ( v1 - unitPosition ) / 10 );
	return v1;
}

// Rule 2: Boids try to keep a small distance away from other objects (including other boids).
Position rule2(Unitset units, int unitID)
{
	Position v2 = 0;
	Position unitPosition = 0;

	for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( unitID == i->getID() )
		{
			unitPosition = i->getPosition();
		}
	}
	for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( unitID != i->getID() )
		{ 
			if ( i->getPosition().getDistance(unitPosition) < 40 )
			{
				v2 -= ( i->getPosition() - unitPosition );
			}
		}
	}

	return v2;
}

// Rule 3: Boids try to match velocity with near boids.
Position rule3(Unitset units, int unitID)
{
	Position v3 = 0;
	Position unitVelocity = 0;
	int total = units.size() - 1;
	for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( unitID == i->getID() )
		{
			unitVelocity.x = (int)ceil( i->getVelocityX() );
			unitVelocity.y = (int)ceil( i->getVelocityY() );
		}
		else
		{
			v3.x += ( (int)ceil( i->getVelocityX() ) / total );
			v3.y += ( (int)ceil( i->getVelocityY() ) / total );
		}
	}
	v3 = ( ( v3 - unitVelocity ) / 8 );
	return v3;
}

// Rule 4: Tendency towards a particular place
Position rule4(Position unitPosition, Position place)
{
	Position v4 = 0;
	v4 = ( ( place - unitPosition ) / 10 );
	return v4;
}