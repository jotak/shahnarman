x = 0
y = 0
texture = "chaos_tower"
attractAI = 1

function getName()
	if language == "french" then
		return "Tour du chaos"
	else
		return "Tower of chaos"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Des chevaux frais"
	else
		return "Fresh horses"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité qui termine son tour sur la Tour du chaos disparaît pendant un temps aléatoire, puis peut soit mourir, soit revenir avec un fort bonus aléatoire. La tour du chaos disparaît ensuite."
	else
		return "The unit that ends its turn on the Tower of chaos disappears for a while, and either dies or comes back with a strong random bonus. Then the Chaos tower disappears."
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
	return 1
end

this = nil
function onCreate(id)
	math.randomseed(os.time())
	this = id
	attachAsGlobal()
end

_player = nil
_unit = nil
_nbTurns = 0
function onNewTurn()
	if _unit == nil then
		i = 0
		choices = {}
		players = {getPlayersList()}
		for p, player in pairs(players) do
			units = {getUnitsList(player)}
			for u, unit in pairs(units) do
				if isShahmah(player, unit) == 0 then
					unitx, unity = getObjectPosition("unit", player, unit)
					if x == unitx and y == unity then
						i = i + 1
						choices[i] = { player, unit }
					end
				end
			end
		end

		if i > 0 then
			rnd = math.random(i)
			_player = choices[rnd][1]
			_unit = choices[rnd][2]
			_nbTurns = math.random(1, 5)
			attachToUnit(_player, _unit)
			detachFromGlobal()
			changeUnitOwner(_player, _unit, 0)
		end
	end
end

_specialEffect = 0
function onNewUnitTurn_step1(unitargs)
	if _specialEffect > 0 then
		return
	end
	objtype, player, unit = split(unitargs, " ")
	if unit == _unit then
		_nbTurns = _nbTurns - 1
		if _nbTurns == 0 then
			changeUnitOwner(player, unit, _player)
			hideSpecialTile(this)
			_specialEffect = math.random(11)
			dispatchToClients("onSpecialEffect", _specialEffect)
			if _specialEffect <= 6 then
				life = getUnitData(_player, unit, "life")
				damageUnit(_player, unit, life)
			end
		else
			if player ~= 0 then
				changeUnitOwner(player, unit, 0)
			end
		end
	end
end

function onSpecialEffect(val)	
	_specialEffect = val
	return 0	-- no need to reload LUA basic data
end

function getMod_melee(val)
	if _specialEffect == 7 then
		return val + 5
	end
	return val
end

function getMod_endurance(val)
	if _specialEffect == 8 then
		return val + 5
	end
	return val
end

function getMod_armor(val)
	if _specialEffect == 9 then
		return val + 3
	end
	return val
end

function getMod_speed(val)
	if _specialEffect == 10 then
		return val + 8
	end
	return val
end

function getMod_range(val)
	if _specialEffect == 11 then
		return val + 3
	end
	return val
end
