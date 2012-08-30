cost = {0,0,0,2}
icon = "mutation"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Mutation"
	else
		return "Mutation"
	end
end

_charac = 1
_modif = 0

function getDescription()
	if _modif == 0 then
		if language == "french" then
			return "L'unité visée gagne ou perd aléatoirement des points de mêlée, armure ou vitesse. Une unité alliée a plus de chance d'obtenir des effets bénéfiques, et vice-versa."
		else
			return "Target unit melee, armor or speed is randomly increased or decreased. Friendly units are more likely to be beneficial, contrary to foes."
		end
	else
		-- for the following code, sentence to translate is :
		-- "Mutated unit [won|lost] X [melee|armor|speed] points."
		if language == "french" then
			word = "de mêlée"
			if _charac == 2 then
				word = "d'armure"
			else
				if _charac == 3 then
					word = "de vitesse"
				end
			end
			word2 = "gagné"
			if _modif < 0 then
				word2 = "perdu"
			end
			return "L'unité mutée a "..word2.." ".._modif.." points "..word.."."
		else
			word = "melee"
			if _charac == 2 then
				word = "armor"
			else
				if _charac == 3 then
					word = "speed"
				end
			end
			word2 = "won"
			if _modif < 0 then
				word2 = "lost"
			end
			return "Mutated unit "..word2.." ".._modif.." "..word.." points."
		end
	end
end

function onCast()
	selectTarget("unit", "")
end

function onResolve(params, caster)
	player, unit = split(params, " ")
	attachToUnit(player, unit)
	_charac = math.random(3)
	_modif = math.random(4)
	if _modif == 4 then
		_modif = -1
	end
	if 0+player ~= caster then
		_modif = -_modif
	end
	dispatchToClients("onMutationDone", _charac, _modif)
end

function onMutationDone(charac, modif)
	_charac = charac
	_modif = modif
	return 1	-- to take into account new description
end

function getMod_melee(i)
	if charac == 1 then
		return i + modif
	end
	return i
end

function getMod_armor(i)
	if charac == 2 then
		return i + modif
	end
	return i
end

function getMod_speed(i)
	if charac == 3 then
		return i + modif
	end
	return i
end
