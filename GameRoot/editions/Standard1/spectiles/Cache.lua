x = 0
y = 0
texture = "Cache"
attractAI = 1

function getName()
	if language == "french" then
		return "Cache"
	else
		return "Hidden place"
	end
end

function getDescription()
	if language == "french" then
		return "Les caches peuvent renfermer un trésor... mais attention aux gardes!"
	else
		return "The hidden places can contain some treasure... but beware the guards!"
	end
end

function init(posx, posy)
	x = posx
	y = posy
end

function getMapPos()
	return x, y
end

function isAllowedOn(terrain)
	if terrain == "sea" then
		return 0
	end
	return 1
end

difficulty = 1
found = 0
this = nil
function onCreate(id)
	this = id
	attachAsGlobal()
	rnd = math.random(2)
	if rnd == 1 then
		rnd = math.random(5)
		difficulty = 2*rnd
		for i = 1, rnd do
			summon(0, x, y, "HellHounds")
		end
	else
		rnd = math.random(3, 8)
		difficulty = 1.5*rnd
		for i = 1, rnd do
			summon(0, x, y, "WarBears")
		end
	end
end

function onNewTurn()
	chosenPlayer = "-1"
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			unitx, unity = getObjectPosition("unit", player, unit)
			if x == unitx and y == unity then
				if player == "0" then		-- neutral player is there => nobody get the treasure!
					chosenPlayer = "0"
					break
				end
				if chosenPlayer == "-1" then	-- a player is here, he might get the treasure
					chosenPlayer = player
					break
				end
				-- at least 2 players are there => nobody get the treasure!
				chosenPlayer = "0"
				break
			end
		end
		if chosenPlayer == "0" then
			break
		end
	end
	if 0+chosenPlayer > 0 then
		detachFromGlobal()
		hideSpecialTile(this)
		difficulty = 10
		if difficulty < 4 then
			rnd = math.random(3)
			if rnd == 1 then
				addGoldToPlayer(chosenPlayer, math.random(2, 10))
			elseif rnd == 2 then
				addSpellToPlayer(chosenPlayer, "Fireball")
			else
				addSpellToPlayer(chosenPlayer, "DivineProtection")
			end
		elseif difficulty < 9 then
			rnd = math.random(3)
			if rnd == 1 then
				addGoldToPlayer(chosenPlayer, math.random(11, 50))
			elseif rnd == 2 then
				addSpellToPlayer(chosenPlayer, "Necrosis")
			else
				addSpellToPlayer(chosenPlayer, "Volcano")
--				addArtifactToPlayer(chosenPlayer, "Dwarven_axe")
			end
		else
			rnd = math.random(3)
			if rnd == 1 then
				addGoldToPlayer(chosenPlayer, math.random(51, 100))
			elseif rnd == 2 then
				addSpellToPlayer(chosenPlayer, "Rationalism")
				addSpellToPlayer(chosenPlayer, "Rationalism")
				addSpellToPlayer(chosenPlayer, "RaiseUndead")
				addSpellToPlayer(chosenPlayer, "RaiseUndead")
			else
				addGoldToPlayer(chosenPlayer, math.random(11, 50))
--				addArtifactToPlayer(chosenPlayer, "Dwarven_axe")
--				addShahmahToPlayer(chosenPlayer, "Dark_Maggot_Master")
			end
		end
	end
end
