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

Position nullPosition, flagMarines, flagFirebat, flagMedic, flagScv;
PositionOrUnit flag1, flag2, flag3, flag4;

void boidsPatrolFlag(Unitset units, PositionOrUnit flag1, PositionOrUnit flag2);

Position rule1(Unitset units, int unitID);
Position rule2(Unitset units, int unitID);
Position rule3(Unitset units, int unitID);
Position rule4(Position unitPosition, Position place);
Position rule5(Unit unit1, Position place);

// Stats
long int distanceMarines, distanceFirebat, distanceMedic, distanceScv;

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
	firebat = NULL;
	medic = NULL;
	scv = NULL;

	nullPosition = NULL;
	flag1 = nullPosition;
	flag2 = nullPosition;
	flag3 = nullPosition;
	flag4 = nullPosition;
	flagMarines = nullPosition;
	flagFirebat = nullPosition;
	flagMedic = nullPosition;
	flagScv = nullPosition;
	
	// Stats
	distanceMarines = 0;
	distanceFirebat = 0;
	distanceMedic = 0;
	distanceScv = 0;
	
	if (Broodwar->isReplay())
    {
		Broodwar << "The following players are in this replay:" << std::endl;;
		Playerset players = Broodwar->getPlayers();
		for(Playerset::iterator p = players.begin(); p != players.end(); ++p )
		{
			if ( !p->getUnits().empty() && !p->isNeutral() )
				Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
		}

	
		// Save distance
		/*
		Unitset allUnits = Broodwar->self()->getUnits();
		for ( Unitset::iterator i = allUnits.begin(); i != allUnits.end(); ++i )
		{
			if ( !i->getType().isPowerup() )
			{
				if ( i->getType() == BWAPI::UnitTypes::Terran_Marine )
				{
					distanceMarines += i->getDistance(nullPosition);
				}
				else if ( i->getType() == BWAPI::UnitTypes::Terran_Firebat )
				{
					distanceFirebat += i->getDistance(nullPosition);
				}
				else if ( i->getType() == BWAPI::UnitTypes::Terran_Medic )
				{
					distanceMedic += i->getDistance(nullPosition);
				}
				else if ( i->getType() == BWAPI::UnitTypes::Terran_SCV )
				{
					distanceScv += i->getDistance(nullPosition);
				}
			}
		}
		*/

		//Broodwar->drawTextScreen(5, 26*0, "M - %f", distanceMarines);
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
			  }
			  else if (!flag2.isUnit())
			  {
				  flag2 = (*i);
			  }
			  else if (!flag3.isUnit())
			  {
				  flag3 = (*i);
			  }
			  else if (!flag4.isUnit())
			  {
				  flag4 = (*i);
			  }
		  }
		  else if ( i->getType() == BWAPI::UnitTypes::Terran_Marine )
		  {
			  marines.insert((*i));
		  }
		  else if ( i->getType() == BWAPI::UnitTypes::Terran_Firebat )
		  {
			  firebat.insert((*i));
		  }
		  else if ( i->getType() == BWAPI::UnitTypes::Terran_Medic )
		  {
			  medic.insert((*i));
		  }
		  else if ( i->getType() == BWAPI::UnitTypes::Terran_SCV )
		  {
			  scv.insert((*i));
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

	  boidsPatrolFlag(marines, flag3, flag2);
	  boidsPatrolFlag(firebat, flag4, flag1);
	  boidsPatrolFlag(medic, flag2, flag3);
	  boidsPatrolFlag(scv, flag1, flag4);

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

void boidsPatrolFlag(Unitset units, PositionOrUnit flag1, PositionOrUnit flag2)
{
	if ( units.size()>0 )
	{
		Position v1, v2, v3, v4, v5, newPosition, unitVelocity, flagPosition;

		if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_Marine )
		{
			flagPosition = flagMarines;
		}
		else if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_Firebat )
		{
			flagPosition = flagFirebat;
		}
		else if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_Medic )
		{
			flagPosition = flagMedic;
		}
		else if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_SCV )
		{
			flagPosition = flagScv;
		}

		if ( units.getPosition().getDistance(flag1.getPosition()) < 70 )
		{
			flagPosition = flag2.getPosition();
		}
		else if ( units.getPosition().getDistance(flag2.getPosition()) < 70 )
		{
			flagPosition = flag1.getPosition();
		}
		else if ( flagPosition.x == 0 && flagPosition.y == 0 )
		{
			if ( units.getPosition().getDistance(flag1.getPosition()) <  units.getPosition().getDistance(flag2.getPosition()) )
			{
				flagPosition = flag1.getPosition();
			}
			else
			{
				flagPosition = flag2.getPosition();
			}
		}

		for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
		{
			// Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
			v1 = rule1( units, i->getID() );
			// Rule 2: Boids try to keep a small distance away from other objects (including other boids).
			v2 = rule2( units, i->getID() );
			// Rule 3: Boids try to match velocity with near boids.
			v3 = rule3( units, i->getID() );
		
			// Rule 4: Tendency towards a particular place
			v4 = rule4( i->getPosition(), flagPosition );
			// Rule 5: Tendency to avoid another units
			v5 = rule5( (*i), flagPosition );
		
			unitVelocity.x = (int)ceil(i->getVelocityX());
			unitVelocity.y = (int)ceil(i->getVelocityY());
			unitVelocity = ( unitVelocity + ( v1*5 ) + ( v2*1 ) + (v3*2 ) + ( v4*4 ) - ( v5*2 ) ) / 1;

			if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_Marine )
			{
				Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v1*5, Colors::Yellow);
				//Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v2, Colors::Red);
				Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v3*2, Colors::Blue);
				Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v4*4, Colors::Purple);
				//Broodwar->drawLineMap(i->getPosition(), i->getPosition()+v5*2, Colors::Orange);
				Broodwar->drawLineMap(i->getPosition(), i->getPosition()+unitVelocity, Colors::Green);
			}

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
			if ( !i->isStuck() )
			{
				i->rightClick(newPosition);
				//Broodwar->drawLineMap(i->getPosition(), newPosition, Colors::Green);
				//Broodwar->drawDotMap(newPosition.x, newPosition.y, Colors::Blue);
			}
		}

		if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_Marine )
		{
			flagMarines = flagPosition;
		}
		else if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_Firebat )
		{
			flagFirebat = flagPosition;
		}
		else if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_Medic )
		{
			flagMedic = flagPosition;
		}
		else if ( units.begin()->getType() == BWAPI::UnitTypes::Terran_SCV )
		{
			flagScv = flagPosition;
		}
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
	/*
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
			if ( i->getPosition().getDistance(unitPosition) < 50 )
			{
				v2 -= ( i->getPosition() - unitPosition );
			}
		}
	}
	*/
	Unitset allUnits = NULL;
	for ( Unitset::iterator i = units.begin(); i != units.end(); ++i )
	{
		if ( unitID == i->getID() )
		{
			allUnits = i->getUnitsInRadius(50, BWAPI::Filter::IsOwned);
			for ( Unitset::iterator j = allUnits.begin(); j != allUnits.end(); ++j )
			{
				v2 -= ( j->getPosition() - i->getPosition() );
			}
			break;
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
	v3 = ( ( v3 - unitVelocity ) );
	
	return v3;
}

// Rule 4: Tendency towards a particular place
Position rule4(Position unitPosition, Position place)
{
	Position v4 = 0;
	v4 = place - unitPosition;
	if ( ( v4.x < -10 && v4.y < -10 ) || ( v4.x > 10 && v4.y > 10 ) )
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
	Position unitDistance = 0;
	Position totalDistance = 0;
	int total = 0;
	Unitset avoidUnits;

	avoidUnits = unit1->getUnitsInRadius(120, !BWAPI::Filter::IsPowerup && BWAPI::Filter::IsOwned && BWAPI::Filter::GetType != unit1->getType());
	total = avoidUnits.size();
	for ( Unitset::iterator i = avoidUnits.begin(); i != avoidUnits.end(); ++i )
	{
		totalDistance += ( ( i->getPosition() - unitPosition ) / total );
	}
	v5.x = totalDistance.y;
	v5.y = totalDistance.x;
	//v5 = totalDistance;
	return v5;
}