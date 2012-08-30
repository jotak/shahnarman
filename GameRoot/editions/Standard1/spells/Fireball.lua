cost = {0,0,0,3}
icon = "fireball"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Boule de feu"
	else
		return "Fireball"
	end
end

function getDescription()
	if language == "french" then
		return "Une boule de feu inflige 4 points de dégâts à une unité."
	else
		return "A fireball deals 4 damages to unit."
	end
end

function onCast()
	selectTarget("unit", "")
end

function onResolve(params)
	player, unit = split(params, " ")
	damageUnit(player, unit, 4)
end
