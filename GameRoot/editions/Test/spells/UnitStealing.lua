cost = {0,0,3,0}
icon = "unitstealing"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Vol d'unit�"
	else
		return "Unit stealing"
	end
end

function getDescription()
	if language == "french" then
		return "L'unit� cibl�e passe sous votre contr�le."
	else
		return "Take control of target unit."
	end
end

function onCast()
	selectTarget("unit", "opponent not_shahmah")
end

function onResolve(params, caster)
	player, unit = split(params, " ")
	changeUnitOwner(player, unit, caster)
end
