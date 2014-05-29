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
bool show_bullets;
bool show_visibility_data;

Unitset marines, firebat, medic, scv;

Position nullPosition;

void boids(Unitset units);

Position rule1(Unitset units, int unitID);
Position rule2(Unitset units, int unitID);
Position rule3(Unitset units, int unitID);
Position rule4(Position unitPosition, Position place);
Position rule5(Unit unit1, Position place);

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
	
	marines = NULL;

	nullPosition = NULL;
	
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
		  if ( i->getType() == BWAPI::UnitTypes::Terran_Marine )
		  {
			  marines.insert((*i));
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
		}
      }

      if (show_bullets)
        drawBullets();

      if (show_visibility_data)
        drawVisibilityData();

      //drawStats();
      //Broodwar->drawTextScreen(300,0,"FPS: %f",Broodwar->getAverageFPS());

	  boids(marines);

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

void boids(Unitset units)
{
	if ( units.size()>0 )
	{
		Position v1, v2, v3, v4, v5, newPosition, unitVelocity, flagPosition;

		int unitID = 0;
		Position unitPosition = 0;
		int total = units.size() - 1;
		Unitset allUnits = NULL;
		int count = 0;
		int unitFollow = units.begin()->getID();
		Position unitFollowPosition = units.begin()->getPosition();

		Broodwar->drawLineMap(units.begin()->getPosition(), Broodwar->getMousePosition(), Colors::Green);
		
		for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
		{
			unitID = i->getID();
			unitPosition = 0;
			allUnits = NULL;
			v1 = 0;
			v2 = 0;
			v3 = 0;
			unitVelocity = 0;
			for ( Unitset::iterator j = units.begin(); j != units.end(); ++j )
			{
				if ( unitID == j->getID() )
				{
					unitPosition = j->getPosition();
					allUnits = j->getUnitsInRadius(50, !BWAPI::Filter::IsPowerup && BWAPI::Filter::IsOwned);
					for ( Unitset::iterator k = allUnits.begin(); k != allUnits.end(); ++k )
					{
						v2 -= ( k->getPosition() - unitPosition );
					}
					unitVelocity.x = (int)ceil( j->getVelocityX() );
					unitVelocity.y = (int)ceil( j->getVelocityY() );
				}
				else
				{
					v1 += ( j->getPosition() / total );
					v3.x += ( (int)ceil( j->getVelocityX() ) / total );
					v3.y += ( (int)ceil( j->getVelocityY() ) / total );
				}
			}
			v1 = ( ( v1 - unitPosition ) / 10 );
			v3 = ( ( v3 - unitVelocity ) );
			v4 = rule4( i->getPosition(), unitFollowPosition );

			// Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
			// Rule 2: Boids try to keep a small distance away from other objects (including other boids).
			// Rule 3: Boids try to match velocity with near boids.
			// Rule 4: Tendency towards a particular place
			//v4 = rule4( i->getPosition(), flagPosition );
			// Rule 5: Tendency to avoid another units
			//v5 = rule5( (*i), flagPosition );
		
			unitVelocity.x = (int)ceil(i->getVelocityX());
			unitVelocity.y = (int)ceil(i->getVelocityY());
			unitVelocity = unitVelocity + ( v1*1 ) + ( v2*4 ) + ( v3*1 ) + ( v4*1 );
			
			//unitVelocity = ( unitVelocity + ( v1*5 ) + ( v2*1 ) + (v3*2 ) + ( v4*0 ) - ( v5*0 ) ) / 1;

			//Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v1*1, Colors::Yellow);
			//Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v2, Colors::Red);
			//Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v3*2, Colors::Blue);
			//Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v4*4, Colors::Purple);
			//Broodwar->drawLineMap(i->getPosition(), i->getPosition()-v5*2, Colors::Orange);
			//Broodwar->drawLineMap(i->getPosition(), i->getPosition()+unitVelocity, Colors::Green);
			int limitVelocity = 100;
			if ( unitVelocity.x > limitVelocity )
			{
				unitVelocity.x = limitVelocity;
			}
			else if ( unitVelocity.x < -limitVelocity )
			{
				unitVelocity.x = -limitVelocity;
			}
			if ( unitVelocity.y > limitVelocity )
			{
				unitVelocity.y = limitVelocity;
			}
			else if ( unitVelocity.y < -limitVelocity )
			{
				unitVelocity.y = -limitVelocity;
			}
			
			newPosition = i->getPosition() + unitVelocity;
			if ( unitFollow != i->getID() )
			{
				i->rightClick(newPosition);
			}
			count++;
			if ( count%35 == 0 )
			{
				BWAPI::BWAPIClient.update();
			}
			//Broodwar->drawLineMap(i->getPosition(), newPosition, Colors::Green);
			//Broodwar->drawDotMap(newPosition.x, newPosition.y, Colors::Blue);
		}
		Broodwar->drawTextScreen(300, 26,"Units per Frame: %d ", Broodwar->getAPM());
	}
}

// Rule 4: Tendency towards a particular place
Position rule4(Position unitPosition, Position place)
{
	Position v4 = 0;
	v4 = place - unitPosition;
	if ( ( v4.x < -40 && v4.y < -40 ) || ( v4.x > 40 && v4.y > 40 ) )
	{
		v4 = v4 / 10;
	}
	
	return v4;
}

// Rule 5: Tendency to avoid another units
Position rule5(Unit unit1, Position place)
{
	Position v5 = 0;
	Position unitPosition = unit1->getPosition();
	Position totalDistance = 0;
	int total = 0;
	Unitset avoidUnits;
	double unitDistance = 0;
	unitDistance = place.getDistance(unit1->getPosition());
	
	avoidUnits = unit1->getUnitsInRadius(120, !BWAPI::Filter::IsPowerup && BWAPI::Filter::IsOwned && BWAPI::Filter::GetType != unit1->getType());
	total = avoidUnits.size();

	for ( Unitset::iterator i = avoidUnits.begin(); i != avoidUnits.end(); ++i )
	{
		if ( place.getDistance(i->getPosition()) < ( unitDistance - 10 ) )
		{
			/*
			if ( unit1->getType() == BWAPI::UnitTypes::Terran_Marine )
			{
				Broodwar->drawLineMap(i->getPosition(), place, Colors::White);
				Broodwar->drawLineMap(i->getPosition(), unitPosition, Colors::White);
				Broodwar->drawLineMap(place, unitPosition, Colors::White);
			}
			*/
			totalDistance += ( ( i->getPosition() - unitPosition ) / total );
		}
	}
	v5.x = totalDistance.y;
	v5.y = totalDistance.x;
	//v5 = totalDistance;
	return v5;
}