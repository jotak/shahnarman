cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Résurrection"
	else
		return "Resurrect"
	end
end

function getDescription()
	if language == "french" then
		return "Ressusciter l'unité ciblée en la mettant sous votre contrôle."
	else
		return "Resurrect target unit and put it under your control."
	end
end

function onCast()
	selectTarget("dead_unit", "")
end

function onResolve(params, caster)
	player, unit = split(params, " ")
	resurrect(player, unit)
	changeUnitOwner(player, unit, caster)
end
