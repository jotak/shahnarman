cost = {0,0,3,0}
icon = "unitstealing"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Vol d'unité"
	else
		return "Unit stealing"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité ciblée passe sous votre contrôle."
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
