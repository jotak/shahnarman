cost = {0,0,0,2}
icon = "lbolt"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Etincelles"
	else
		return "Sparkles"
	end
end

function getDescription()
	if language == "french" then
		return "Les étincelles touchent 3 unités, leur infligeant 1 point de dégât chacune."
	else
		return "The sparkles hit 3 units, dealing 1 damage each."
	end
end

function onCast()
	selectTarget("unit", "inrange", "", "onFirstSelected")
end

function onFirstSelected()
	selectTarget("unit", "inrange", "", "onSecondSelected")
end

function onSecondSelected()
	selectTarget("unit", "inrange")
end

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolve(params)
	player1, unit1, player2, unit2, player3, unit3 = split(params, " ")
	damageUnit(player1, unit1, 1)
	damageUnit(player2, unit2, 1)
	damageUnit(player3, unit3, 1)
end
