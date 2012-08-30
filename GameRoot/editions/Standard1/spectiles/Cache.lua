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
	print("onCreate 1", id)	-- note : debug en place car plantage parfois en début de partie... je garde ce debug tant que je suis pas sûr que le bug soit parti
	this = id
	print("onCreate 2")
	attachAsGlobal()
	print("onCreate 3")
	rnd = math.random(2)
	if rnd == 1 then
		rnd = math.random(5)
	print("onCreate 4")
		difficulty = 2*rnd
		for i = 1, rnd do
			summon(0, x, y, "Djinn_tres_tres_mechant")
		end
	print("onCreate 5")
	else
		rnd = math.random(3, 8)
	print("onCreate 6")
		difficulty = 1.5*rnd
		for i = 1, rnd do
			summon(0, x, y, "Troll_aquatique_farceur")
		end
	print("onCreate 7")
	end
end

first = 1
function onNewTurn()
if first == 1 then
print("onNewTurn 1", this)
end
	chosenPlayer = "-1"
	players = {getPlayersList()}
if first == 1 then
print("onNewTurn 2")
end
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
if first == 1 then
print("onNewTurn 3")
end
		for u, unit in pairs(units) do
			unitx, unity = getObjectPosition("unit", player, unit)
			if x == unitx and y == unity then
				if player == "0" then		-- neutral player is there => nobody get the treasure!
print("  neutral player is there => nobody get the treasure!")
					chosenPlayer = "0"
					break
				end
				if chosenPlayer == "-1" then	-- a player is here, he might get the treasure
print("  a player is here, he might get the treasure")
					chosenPlayer = player
					break
				end
				-- at least 2 players are there => nobody get the treasure!
print("  at least 2 players are there => nobody get the treasure!")
				chosenPlayer = "0"
				break
			end
		end
if first == 1 then
print("onNewTurn 4")
end
		if chosenPlayer == "0" then
			break
		end
if first == 1 then
print("onNewTurn 5")
end
	end

if first == 1 then
print("onNewTurn 5.5", 0+chosenPlayer)
end
	if 0+chosenPlayer > 0 then
if first == 1 then
print("onNewTurn 6")
end
		detachFromGlobal()
		hideSpecialTile(this)
if first == 1 then
print("onNewTurn 7")
end
		difficulty = 10
		if difficulty < 4 then
			rnd = math.random(3)
			if rnd == 1 then
				addGoldToPlayer(chosenPlayer, math.random(2, 10))
			elseif rnd == 2 then
				addSpellToPlayer(chosenPlayer, "MagicCircle")
			else
				addSpellToPlayer(chosenPlayer, "FrozenLand")
			end
		elseif difficulty < 9 then
			rnd = math.random(3)
			if rnd == 1 then
				addGoldToPlayer(chosenPlayer, math.random(11, 50))
			elseif rnd == 2 then
				addSpellToPlayer(chosenPlayer, "LightningBolt")
			else
				addArtifactToPlayer(chosenPlayer, "Dwarven_axe")
			end
		else
			rnd = math.random(3)
			if rnd == 1 then
				addGoldToPlayer(chosenPlayer, math.random(51, 100))
			elseif rnd == 2 then
				addSpellToPlayer(chosenPlayer, "LightningBolt")
				addSpellToPlayer(chosenPlayer, "LightningBolt")
				addSpellToPlayer(chosenPlayer, "Teleport")
				addSpellToPlayer(chosenPlayer, "SummonDjinnTresTresMechant")
			else
				addGoldToPlayer(chosenPlayer, math.random(11, 50))
				addArtifactToPlayer(chosenPlayer, "Dwarven_axe")
				addShahmahToPlayer(chosenPlayer, "Dark_Maggot_Master")
			end
		end


if first == 1 then
print("onNewTurn 8")
end
	end
if first == 1 then
print("onNewTurn 9")
end
first = 0
end
