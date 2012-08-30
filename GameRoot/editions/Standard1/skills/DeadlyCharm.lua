childEffectsCount = 1
childEffectsCost = {{0,4,0,0}}	-- 4 law
childEffectsIcon = {"ane"}
icon = "ane"

function getName()
	if language == "french" then
		return "Charme mortel"
	else
		return "Deadly charm"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Charme"
	else
		return "Charm"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : 4|. L'unité ciblée perd toute capacité ce tour-ci."
	else
		return "Activation cost: 4|. Target unit loose all its skills this turn."
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("unit", "")
	end
end

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolveChild(effect, params)
	if effect == 1 then
		player, unit = splitint(params, " ")
		skills = {getSkillsList(player, unit)}
		table.foreach(skills, function(i, skill)
			deactivateSkill(player, unit, skill)
		end)
	end
end
